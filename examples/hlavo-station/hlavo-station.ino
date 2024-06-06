#include <Every.h>

#define PIN_ON 47 // napajeni !!!

/** TIMERS */
// times in milliseconds, L*... timing level
Every timer_L1(1000); // fine timer - humidity, temperature, ...
// L2 - hardware timer with L2_WEATHER_PERIOD in seconds (TEST 10 s, RUN 60 s)
#define L2_WEATHER_PERIOD 10
Every timer_L3(30000); // coarse timer - PR2 - TEST 30 s
// Every timer_L3(900000); // coarse timer - PR2 - RUN 15 min

/*********************************************** SD CARD ***********************************************/
// SD card IO
#include "SD.h"
// file handling
#include "file_info.h"
// SD card pin
#define SD_CS_PIN 10
#define data_meteo_filename "/meteo.csv"


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

#define PR2_DATA_PIN 4         /*!< The pin of the SDI-12 data bus */
PR2Comm pr2(PR2_DATA_PIN, 0);  // (data_pin, verbose)
const uint8_t n_pr2_sensors = 2;
const uint8_t pr2_addresses[n_pr2_sensors] = {0,1};  // sensor addresses on SDI-12
PR2Reader pr2_readers[2] = {        // readers enable reading all sensors without blocking loop
  PR2Reader(pr2, pr2_addresses[0]),
  PR2Reader(pr2, pr2_addresses[1])
};
const char* data_pr2_filenames[n_pr2_sensors] = {"/pr2_a0.csv", "/pr2_a1.csv"};

uint8_t iss = 0;  // current sensor reading
bool pr2_all_finished = false;

/****************************************** DATA COLLECTION ******************************************/
// L1 timer data buffer
float fineDataBuffer[NUM_FINE_VALUES][FINE_DATA_BUFSIZE];
int num_fine_data_collected = 0;

// L3 timer data buffer
MeteoData meteoDataBuffer[METEO_DATA_BUFSIZE];
int num_meteo_data_collected = 0;

// collects data at fine interval
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

  // Serial.printf("%d:   %.2f,  %.2f,  %.0f,  %.3f\n", num_fine_data_collected,
  //   fineDataBuffer[0][i], fineDataBuffer[1][i], fineDataBuffer[2][i], fineDataBuffer[3][i]);

  num_fine_data_collected++;
}

// save the meteo data to buffer
void meteo_data_collect()
{
  DateTime dt = rtc_clock.now();
  Serial.printf("DateTime: %s. Buffering MeteoData.\n", dt.timestamp().c_str());

  // should not happen
  if(num_meteo_data_collected >= METEO_DATA_BUFSIZE)
  {
    Serial.printf("Warning: Reached maximal buffer size for meteo data (%d)\n", num_meteo_data_collected);
    all_data_write();
    num_meteo_data_collected = 0;
  }

  // MeteoData &data = meteoDataBuffer[num_meteo_data_collected];
  MeteoData data;
  data.datetime = dt;
  data.compute_statistics(fineDataBuffer, NUM_FINE_VALUES, num_fine_data_collected);

  if (weather.gotData())
  {
    data.wind_direction = weather.getDirection();
    data.wind_speed = weather.getSpeed();
    data.raingauge = weather.getRain();
    // Serial.printf("%d:   %.2f,  %d  %.2f,  %d  %.2f\n", num_meteo_data_collected,
    // data.wind_direction, weather.getSpeedTicks(), data.wind_speed, weather.getRainTicks(), data.raingauge);
  }

  // write data into buffer
  meteoDataBuffer[num_meteo_data_collected] = data;
  num_meteo_data_collected++;

  // start over from the beginning of buffer
  num_fine_data_collected = 0;
}

// save the meteo data buffer to CSV
void all_data_write()
{
  // FileInfo datafile(SD, data_meteo_filename);
  char csvLine[100];

  File file = SD.open(data_meteo_filename, FILE_APPEND);
  if(!file){
      Serial.println("Failed to open file for appending");
  }
  else
  {
    for(int i=0; i<num_meteo_data_collected; i++)
    {
      bool res = file.print(meteoDataBuffer[i].dataToCsvLine(csvLine));
      if(!res){
          Serial.println("Append failed");
      }
    }
  }
  file.close();

  // start over from the beginning of buffer
  num_meteo_data_collected = 0;
}

void collect_and_write_PR2(uint8_t sid)
{
  PR2Data data;
  char csvLine[200];

  uint8_t address = pr2_addresses[sid];
  float values[7];
  uint8_t n_values = 0;
  String sensorResponse = "";

  Serial.printf("--------------- collect PR2 [%d] a%d\n", sid, address);
  data.datetime = rtc_clock.now();

  sensorResponse = pr2.measureRequestAndRead("C", address, values, &n_values);
  pr2.print_values("permitivity", values, n_values);
  data.setPermitivity(values, n_values);

  sensorResponse = pr2.measureRequestAndRead("C1", address, values, &n_values);
  pr2.print_values("soil moisture mineral", values, n_values);
  data.setSoilMoisture(values, n_values);

  // sensorResponse = pr2.measureConcurrent("C2", address, values, &n_values);
  // pr2.print_values("soil moisture organic", values, n_values);

  // sensorResponse = pr2.measureConcurrent("C3", address, values, &n_values);
  // pr2.print_values("soil moisture mineral (%)", values, n_values);

  // sensorResponse = pr2.measureConcurrent("C4", address, values, &n_values);
  // pr2.print_values("soil moisture mineral (%)", values, n_values);

  // sensorResponse = pr2.measureConcurrent("C7", address, values, &n_values);
  // pr2.print_values("millivolts", values, n_values);

  // sensorResponse = pr2.measureConcurrent("C8", address, values, &n_values);
  // pr2.print_values("millivolts uncalibrated", values, n_values);

  sensorResponse = pr2.measureRequestAndRead("C9", address, values, &n_values);
  pr2.print_values("raw ADC", values, n_values);
  data.setRaw_ADC(&values[1], n_values-1);  // skip zero channel

  data.dataToCsvLine(csvLine);

  File file = SD.open(data_pr2_filenames[sid], FILE_APPEND);
  if(!file){
      Serial.printf("Failed to open file for appending: %s\n", data_pr2_filenames[sid]);
  }
  else
  {
    bool res = file.print(data.dataToCsvLine(csvLine));
    if(!res){
        Serial.printf("Append failed: %s\n", data_pr2_filenames[sid]);
    }
  }
  file.close();

  Serial.println("PR2 data written.");
}

void collect_and_write_PR2()
{
  pr2_readers[iss].TryRequest();
  pr2_readers[iss].TryRead();
  if(pr2_readers[iss].finished)
  {
    DateTime dt = rtc_clock.now();
    pr2_readers[iss].data.datetime = dt;
    Serial.printf("DateTime: %s. Writing PR2Data[a%d].\n", dt.timestamp().c_str(), pr2_addresses[iss]);

    File file = SD.open(data_pr2_filenames[iss], FILE_APPEND);
    if(!file){
        Serial.printf("Failed to open file for appending: %s\n", data_pr2_filenames[iss]);
    }
    else
    {
      char csvLine[200];
      pr2_readers[iss].data.dataToCsvLine(csvLine);
      bool res = file.print(csvLine);
      if(!res){
          Serial.printf("Append failed: %s\n", data_pr2_filenames[iss]);
      }
    }
    file.close();
    // Serial.println("PR2 data written.");

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
  // Meteo Data
  {
    FileInfo datafile(SD, data_meteo_filename);
    datafile.remove();
    // datafile.read();
    if(!datafile.exists())
      datafile.write(MeteoData::headerToCsvLine(csvLine));
  }
  // PR2 Data
  for(int i=0; i<n_pr2_sensors; i++)
  {
    FileInfo datafile(SD, data_pr2_filenames[i]);
    datafile.remove();
    // datafile.read();
    if(!datafile.exists())
      datafile.write(PR2Data::headerToCsvLine(csvLine));  
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

  // read value to buffer at fine time scale
  if(timer_L1())
  {
    // Serial.printf("L1 tick\n");
    Serial.println("-------------------------- L1 TICK --------------------------");
    fine_data_collect();
  }

	if (weather.gotData()) {
    // Serial.printf("L2 tick - Weather\n");

    // sensors_event_t humidity, temp; // promenne vlhkost a teplota
    // sht4.getEvent(&humidity, &temp);
    // float light_lux = lightMeter.readLightLevel();

    // DateTime dt = rtc_clock.now();

    // Serial.printf("DateTime: %s\n", dt.timestamp().c_str());
    // Serial.printf("Temperature: %f degC\n", temp.temperature);
    // Serial.printf("Humidity: %f rH\n", humidity.relative_humidity);
    // Serial.printf("Light: %f lx\n", light_lux);

    // Serial.printf("Wind direc adc:  %d\n", weather.getDirAdcValue());
    // Serial.printf("Wind direc deg:  %f\n", weather.getDirection());
    // Serial.printf("Wind speed TICK: %d\n", weather.getSpeedTicks());
    // Serial.printf("Rain gauge TICK: %d\n", weather.getRainTicks());
    // Serial.printf("Battery [V]: %f\n", adc.readVoltage() * DeviderRatio);

    meteo_data_collect();
    weather.resetGotData();
  }

  if(!pr2_all_finished)
    collect_and_write_PR2();

  // read value to buffer at fine time scale
  if(timer_L3())
  {
    // Serial.printf("L3 tick\n");
    all_data_write();

    // collect_and_write_PR2(0);
    // collect_and_write_PR2(1);
    pr2_all_finished = false;

    // TEST read data from CSV
    {
      FileInfo datafile(SD, data_meteo_filename);
      datafile.read();
    }
    for(int i=0; i<n_pr2_sensors; i++)
    {
      FileInfo datafile(SD, data_pr2_filenames[i]);
      datafile.read();
    }
  }
}