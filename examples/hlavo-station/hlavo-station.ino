
/*********************************************** COMMON ***********************************************/
#include <Every.h>

#define PIN_ON 47 // napajeni !!!


/************************************************ RUN ************************************************/
// Switch between testing/setup and long term run.
#define TEST

#ifdef TEST
    /** TIMERS */
    // times in milliseconds, L*... timing level
    Every timer_L1(1000);      // fine timer - humidity, temperature, meteo, ...
    // L2 - hardware timer with L2_WEATHER_PERIOD in seconds
    #define L2_WEATHER_PERIOD 10
    Every timer_L3(30*1000); // coarse timer - PR2 - 30 s
    Every timer_L4(60*1000);  // watchdog timer - 1 min
    #define VERBOSE 1
#else
    /** TIMERS */
    // times in milliseconds, L*... timing level
    Every timer_L1(1000);         // fine timer - humidity, temperature, ...
    // L2 - hardware timer with L2_WEATHER_PERIOD in seconds
    #define L2_WEATHER_PERIOD 60
    Every timer_L3(900*1000);     // coarse timer - PR2 - 15 min
    Every timer_L4(24*3600*1000); // watchdog timer - 24 h
    #define VERBOSE 0
#endif


/*********************************************** SD CARD ***********************************************/
// SD card IO
#include "CSV_handler.h"
// SD card pin
#define SD_CS_PIN 10

/************************************************* RTC *************************************************/
// definice sbernice i2C pro RTC (real time clock)
#define rtc_SDA_PIN 42 // data pin
#define rtc_SCL_PIN 2  // clock pin
#include "clock.h"
Clock rtc_clock(rtc_SDA_PIN, rtc_SCL_PIN);


/*********************************************** BATTERY ***********************************************/
#include "ESP32AnalogRead.h"
ESP32AnalogRead adc;
#define ADCpin 9
#define DeviderRatio 1.7693877551  // Voltage devider ratio on ADC pin 1MOhm + 1.3MOhm


/******************************************* TEMP. AND HUM. *******************************************/
#include "Adafruit_SHT4x.h"
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

#include "BH1750.h"
BH1750 lightMeter;

/****************************************** WHEATHER STATION ******************************************/
#define WINDVANE_PIN A0   // A0 := 1
#define ANEMOMETER_PIN 5
#define RAINGAUGE_PIN 6  // 10 kOhm, pullup
#include "weather_station.h"
#include "meteo_data.h"
char data_meteo_filename[100] = "meteo.csv";
WeatherStation weather(WINDVANE_PIN, ANEMOMETER_PIN, RAINGAUGE_PIN, L2_WEATHER_PERIOD);

// interuption
// ICACHE_RAM_ATTR replaced by IRAM_ATTR (esp and arduino>3.0.0)
void IRAM_ATTR intAnemometer() { weather.intAnemometer(); }
void IRAM_ATTR intRaingauge() { weather.intRaingauge(); }
void IRAM_ATTR intPeriod() { weather.intTimer(); }

/******************************************** PR2 SENSORS ********************************************/
#include <esp_intr_alloc.h>
#include "pr2_comm.h"
#include "pr2_data.h"
#include "pr2_reader.h"

#define PR2_DATA_PIN 4         /*!< The pin of the SDI-12 data bus */
PR2Comm pr2(PR2_DATA_PIN, 0);  // (data_pin, verbose)
const uint8_t n_pr2_sensors = 2;
const uint8_t pr2_addresses[n_pr2_sensors] = {0,1};  // sensor addresses on SDI-12
PR2Reader pr2_readers[2] = {        // readers enable reading all sensors without blocking loop
  PR2Reader(pr2, pr2_addresses[0]),
  PR2Reader(pr2, pr2_addresses[1])
};
char data_pr2_filenames[n_pr2_sensors][100] = {"pr2_a0.csv", "pr2_a1.csv"};

uint8_t iss = 0;  // current sensor reading
bool pr2_all_finished = false;

/****************************************** DATA COLLECTION ******************************************/
// L1 timer data buffer
float fineDataBuffer[NUM_FINE_VALUES][FINE_DATA_BUFSIZE];
int num_fine_data_collected = 0;

// L3 timer data buffer
MeteoData meteoDataBuffer[METEO_DATA_BUFSIZE];
int num_meteo_data_collected = 0;

// collect meteo data at fine interval into a fine buffer of floats
void fine_data_collect()
{
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);
  float light_lux = lightMeter.readLightLevel();
  float battery = adc.readVoltage() * DeviderRatio;

  // should not happen
  if(num_fine_data_collected >= FINE_DATA_BUFSIZE)
  {
    Serial.printf("Warning: Reached maximal buffer size for fine timer (%d)\n", num_fine_data_collected);
    meteo_data_collect();
    num_fine_data_collected = 0;
  }

  int i = num_fine_data_collected;
  fineDataBuffer[0][i] = humidity.relative_humidity;
  fineDataBuffer[1][i] = temp.temperature;
  fineDataBuffer[2][i] = light_lux;
  fineDataBuffer[3][i] = battery;

  if(VERBOSE >= 2)
  {
    Serial.printf("%d:   %.2f,  %.2f,  %.0f,  %.3f\n", num_fine_data_collected,
      fineDataBuffer[0][i], fineDataBuffer[1][i], fineDataBuffer[2][i], fineDataBuffer[3][i]);
  }

  num_fine_data_collected++;
}

// compute statistics over the fine meteo data
// and save the meteo data into buffer of MeteoData
void meteo_data_collect()
{
  DateTime dt = rtc_clock.now();
  Serial.printf("DateTime: %s. Buffering MeteoData.\n", dt.timestamp().c_str());

  // should not happen
  if(num_meteo_data_collected >= METEO_DATA_BUFSIZE)
  {
    Serial.printf("Warning: Reached maximal buffer size for meteo data (%d)\n", num_meteo_data_collected);
    meteo_data_write();
    num_meteo_data_collected = 0;
  }

  MeteoData &data = meteoDataBuffer[num_meteo_data_collected];
  data.datetime = dt;
  data.compute_statistics(fineDataBuffer, NUM_FINE_VALUES, num_fine_data_collected);

  if (weather.gotData())
  {
    data.wind_direction = weather.getDirection();
    data.wind_speed = weather.getSpeed();
    data.raingauge = weather.getRain_ml();
  }

  if(VERBOSE >= 1)
  {
    char line[400];
    data.dataToCsvLine(line);
    Serial.println(line);
  }

  // write data into buffer
  num_meteo_data_collected++;

  // start over from the beginning of buffer
  num_fine_data_collected = 0;
}

// write the meteo data buffer to CSV
void meteo_data_write()
{
  // Fill the base class pointer array with addresses of derived class objects
  DataBase* dbPtr[num_meteo_data_collected];
  for (int i = 0; i < num_meteo_data_collected; i++) {
      dbPtr[i] = &meteoDataBuffer[i];
  }

  CSVHandler::appendData(data_meteo_filename, dbPtr, num_meteo_data_collected);
  // start over from the beginning of buffer
  num_meteo_data_collected = 0;
}

// use PR2 reader to request and read data from PR2
// minimize delays so that it does not block main loop
void collect_and_write_PR2()
{
  pr2_readers[iss].TryRequest();
  pr2_readers[iss].TryRead();
  if(pr2_readers[iss].finished)
  {
    DateTime dt = rtc_clock.now();
    pr2_readers[iss].data.datetime = dt;
    Serial.printf("DateTime: %s. Writing PR2Data[a%d].\n", dt.timestamp().c_str(), pr2_addresses[iss]);

    CSVHandler::appendData(data_pr2_filenames[iss], &(pr2_readers[iss].data));

    pr2_readers[iss].Reset();
    iss++;
    if(iss == 2)
    {
      iss = 0;
      pr2_all_finished = true;
    }
  }
}

/*********************************************** SETUP ***********************************************/
void setup() {
  Serial.begin(115200);
  while (!Serial)
  {
      ; // cekani na Serial port
  }

  Serial.println("Starting HLAVO station setup.");

  // for version over 3.5 need to turn uSUP ON
  Serial.print("set power pin: "); Serial.println(PIN_ON);
  pinMode(PIN_ON, OUTPUT);      // Set EN pin for uSUP stabilisator as output
  digitalWrite(PIN_ON, HIGH);   // Turn on the uSUP power

  // clock setup
  rtc_clock.begin();
  DateTime dt = rtc_clock.now();

  // weather station
  weather.setup(intAnemometer, intRaingauge, intPeriod);

  // battery
  adc.attach(ADCpin); // setting ADC

  // humidity and temperature
  if (! sht4.begin())
  {
    Serial.println("SHT4x not found.");
  }

  sht4.setPrecision(SHT4X_HIGH_PRECISION); // nejvyssi rozliseni
  sht4.setHeater(SHT4X_NO_HEATER); // bez vnitrniho ohrevu

  // Light
  if(!lightMeter.begin())
  {
    Serial.println("BH1750 (light) not found.");
  }

  // PR2
  gpio_install_isr_service( ESP_INTR_FLAG_IRAM);
  Serial.println("Opening SDI-12 for PR2...");
  pr2.begin();
  delay(500);  // allow things to settle
  String si = pr2.requestAndReadData("?I!", false);  // Command to get sensor info
  // Serial.println(si);

  // SD card setup
  pinMode(SD_CS_PIN, OUTPUT);
  // SD Card Initialization
  if (SD.begin()){
      Serial.println("SD card is ready to use.");
  }
  else{
      Serial.println("SD card initialization failed");
      return;
  }

  // Data files setup
  char csvLine[400];
  CSVHandler::createFile(data_meteo_filename,
                            MeteoData::headerToCsvLine(csvLine),
                            dt);
  for(int i=0; i<n_pr2_sensors; i++){
    CSVHandler::createFile(data_pr2_filenames[i],
                              PR2Data::headerToCsvLine(csvLine),
                              dt);
  }

  Serial.println("HLAVO station is running.");
  Serial.println(F("Start loop " __FILE__ " " __DATE__ " " __TIME__));
  Serial.println("=============================================================");

  // synchronize timers after setup
  timer_L3.reset(true);
  timer_L1.reset(true);
}


/*********************************************** LOOP ***********************************************/ 
void loop() {
  
  weather.update();

  // read values to buffer at fine time scale [fine Meteo Data]
  if(timer_L1())
  {
    Serial.println("-------------------------- L1 TICK --------------------------");
    fine_data_collect();
  }

  // read values to buffer at fine time scale [averaged Meteo Data]
	if (weather.gotData()) {
    Serial.println("-------------------------- L2 TICK --------------------------");

    meteo_data_collect();
    weather.resetGotData();
  }

  // read values from PR2 sensors when reading not finished yet
  // and write to a file when last values received
  if(!pr2_all_finished)
    collect_and_write_PR2();

  // request reading from PR2 sensors
  // and write Meteo Data buffer to a file
  if(timer_L3())
  {
    Serial.println("-------------------------- L3 TICK --------------------------");
    meteo_data_write();

    pr2_all_finished = false;

    // TEST read data from CSV
    CSVHandler::printFile(data_meteo_filename);
    for(int i=0; i<n_pr2_sensors; i++){
      CSVHandler::printFile(data_pr2_filenames[i]);
    }

  if(timer_L4())
  {
    Serial.println("-------------------------- L4 TICK --------------------------");
    Serial.printf("\nReboot...\n\n");
    delay(250);
    ESP.restart();
  }
}