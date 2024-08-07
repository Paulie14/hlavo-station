
/*********************************************** COMMON ***********************************************/
#include <Every.h>
#include <Logger.h>
#include <math.h>

#define PIN_ON 47 // napajeni !!!

const char* setup_interrupt = "SETUP INTERRUPTED";

/************************************************ RUN ************************************************/
// Switch between testing/setup and long term run.
#define TEST

#ifdef TEST
    /** TIMERS */
    // times in milliseconds, L*... timing level
    const uint8_t timer_L1_period = 3;      // [s] read water height period
    const uint8_t timer_L2_period = 15;     // [s] date reading timer - PR2
    const uint8_t timer_L4_period = 10*60;  // [s] watchdog timer - 10 min
    #define VERBOSE 1
#else
    /** TIMERS */
    // times in milliseconds, L*... timing level
    const uint8_t timer_L1_period = 30;       // [s] read water height period
    const uint8_t timer_L2_period = 5*60;     // [s] date reading timer - PR2
    const uint8_t timer_L4_period = 24*3600;  // [s] watchdog timer - 24 h
    #define VERBOSE 1
#endif

Every timer_L1(timer_L1_period*1000);     // read water height timer
Every timer_L2(timer_L2_period*1000);     // date reading timer - PR2
Every timer_L4(timer_L4_period*60*1000);  // watchdog timer


/*********************************************** SD CARD ***********************************************/
// SD card IO
#include "CSV_handler.h"
// SD card pin
#define SD_CS_PIN 10

/************************************************* I2C *************************************************/
#include <Wire.h>
#define I2C_SDA_PIN 42 // data pin
#define I2C_SCL_PIN 2  // clock pin

/************************************************* RTC *************************************************/
// definice sbernice i2C pro RTC (real time clock)
// I2C address 0x68
#include "clock.h"
Clock rtc_clock;

/************************************************ FLOW *************************************************/
#include "column_flow_data.h"
#define PUMP_OUT_PIN 21

bool pump_out_finished = true;

Timer timer_outflow(15*1000, false);    // time for pumping water out
const float H_limit = 20;               // [cm] limit water height to release
const uint8_t n_H_avg = 5;              // number of flow samples to average

float H_window[n_H_avg];    // collected water height
uint8_t n_H_collected = 0;    // collected water height
uint8_t n_H_collected_start = 0;    // collected water height

float previous_height = 0;

char data_flow_filename[max_filepath_length] = "column_flow.csv";


/************************************************ RAIN *************************************************/
#define PUMP_IN_PIN 20
bool pump_in_finished = true;
Timer timer_rain(1000, false);    // timer for rain
uint8_t current_rain_idx = 0;
FileInfo current_rain_file(SD,"/current_rain.txt");

class RainRegime{
  public:
    static const float pump_rate;       // [l/min]
    static const float column_radius;   // [m]
    static const float column_cross;    // [m2]

    float rate;   // [mm/h]
    float length; // [h]
    float period; // [h]

    uint8_t trigger_length;
    uint8_t trigger_period;

    DateTime last_rain;

    RainRegime(float rate, float length, float period)
    : rate(rate), length(length), period(period)
    {
    }

    void compute_trigger()
    {
      float pump_rate_m = pump_rate*60;  // [dm3/h]
      float inflow_rate = rate/100 * column_cross*100; // [dm3/h]
      float ratio = inflow_rate/pump_rate_m;

      Logger::printf(Logger::INFO, "Ratio: %g\n", ratio);
      float Tmin = 1.0;
      float Tmax = 3600.0*length/10.0;

      trigger_period = std::round((Tmin+Tmax)/2);
      trigger_length = std::round(trigger_period * ratio);

      Logger::printf(Logger::INFO, "Pump lenght: %d  Pump period: %d\n", trigger_length, trigger_period);
    }
};

const float RainRegime::pump_rate = 1.5;            // [l/min]
const float RainRegime::column_radius = 0.2;        // [m]
const float RainRegime::column_cross = PI * column_radius*column_radius;  // [m2]

RainRegime rain_regimes[2] = {
  RainRegime(0.2, 2, 24), // small daily rain
  RainRegime(2, 0.5, 72)  // downpour every 3 days
};

void saveCurrentRain() {
  char text[100];
  DateTime dt = rain_regimes[current_rain_idx].last_rain;
  snprintf(text, sizeof(text), "%s\n%d", dt.timestamp().c_str(), current_rain_idx);
  current_rain_file.write(text);
}

void loadCurrentRain() {
  File file = SD.open(current_rain_file.getPath());
  if(!file){
      Serial.println("Failed to open file for reading.");
      current_rain_idx = 0;
      rain_regimes[current_rain_idx].last_rain = DateTime((uint32_t)0);
      return;
  }

  // Serial.print("Read from file: ");
  if(file.available()){
      String dt_string = file.readStringUntil('\n');
      DateTime dt(dt_string.c_str());
      uint8_t idx = file.readStringUntil('\n').toInt();
      
      current_rain_idx = idx;
      if(idx >= sizeof(rain_regimes))
      {
        Logger::printf(Logger::ERROR, "Invalid rain regime index: %d\n", idx);
        current_rain_idx = 0;
        rain_regimes[current_rain_idx].last_rain = dt;
      }
      else
        rain_regimes[current_rain_idx].last_rain = dt;
  }
  file.close();
}


/******************************************* TEMP. AND HUM. *******************************************/
#include "Adafruit_SHT4x.h"
#include <Wire.h>  
#include "SparkFunBME280.h"
// https://www.laskakit.cz/senzor-tlaku--teploty-a-vlhkosti-bme280--1m/
// https://github.com/sparkfun/SparkFun_BME280_Arduino_Library/releases
// https://randomnerdtutorials.com/esp32-bme280-arduino-ide-pressure-temperature-humidity/
// set I2C address, default is 0x77, LaskaKit supplies with 0x76
const uint8_t tempSensor_I2C = 0x76;
BME280 tempSensor;


#include "BH1750.h"
// default I2C address 0x23 (set in constructor)
BH1750 lightMeter;

/********************************************* PR2 SENSORS ********************************************/
#include <esp_intr_alloc.h>
#include "pr2_comm.h"
#include "pr2_data.h"
#include "pr2_reader.h"

#define PR2_POWER_PIN 7        // The pin PR2 power
#define PR2_DATA_PIN 4         // The pin of the SDI-12 data bus
PR2Comm pr2(PR2_DATA_PIN, 0);  // (data_pin, verbose)
const uint8_t n_pr2_sensors = 1;
const uint8_t pr2_addresses[n_pr2_sensors] = {3};  // sensor addresses on SDI-12
PR2Reader pr2_readers[1] = {        // readers enable reading all sensors without blocking loop
  PR2Reader(pr2, pr2_addresses[0])
};
char data_pr2_filenames[n_pr2_sensors][max_filepath_length] = {"pr2_a3.csv"};

uint8_t iss = 0;  // current sensor reading
bool pr2_all_finished = false;

Timer timer_PR2_power(2000, false);

/****************************************** DATA COLLECTION ******************************************/


void read_water_height()
{
  float height = 0.123; // TODO: read height from ultrasound
  H_window[n_H_collected] = height;
  n_H_collected++;
  if(n_H_collected == n_H_avg)
    n_H_collected = 0;

  // check height limit, possibly run pump out
  if(height >= H_limit)
  {
    timer_outflow.reset();
    pump_out_finished = false;
    digitalWrite(PUMP_OUT_PIN, HIGH);
    // reset window
    n_H_collected = 0;
    n_H_collected_start = 0;
    previous_height = 0;
  }

  ColumnFlowData data;
  data.pump_in = !pump_in_finished;   // is pump running?
  data.pump_out = !pump_out_finished; // is pump running?

  if(n_H_collected_start < n_H_avg)
  {
    n_H_collected_start++;
    // safe NaN until window is filled
    data.height = std::numeric_limits<float>::quiet_NaN();
    data.flux = std::numeric_limits<float>::quiet_NaN();
  }
  else
  {
    // compute average over H window
    float H_avg = 0;
    for (uint8_t i=0; i<n_H_avg; i++)
    {
      H_avg += H_window[i];
    }
    H_avg /= n_H_avg;
    data.height = H_avg;
    // flux is difference of heights
    data.flux = (previous_height - H_avg) / timer_L1_period;
    previous_height = H_avg;
  }

  // write data
  CSVHandler::appendData(data_pr2_filenames[iss], &data);
}

// compute statistics over the fine meteo data
// and save the meteo data into buffer of MeteoData
void meteo_data_collect()
{
  // sensors_event_t humidity, temp;
  // sht4.getEvent(&humidity, &temp);
  
  // // should not happen
  // if(num_fine_data_collected >= FINE_DATA_BUFSIZE)
  // {
  //   Logger::printf(Logger::ERROR, "Warning: Reached maximal buffer size for fine timer (%d)\n", num_fine_data_collected);
  //   meteo_data_collect();
  //   num_fine_data_collected = 0;
  // }

  // int i = num_fine_data_collected;
  // fineDataBuffer[0][i] = humidity.relative_humidity;
  // fineDataBuffer[1][i] = temp.temperature;

  // if(VERBOSE >= 1)
  // {
  //   Serial.printf("        %d:  Hum. %.2f, Temp. %.2f\n", num_fine_data_collected,
  //     fineDataBuffer[0][i], fineDataBuffer[1][i]);
  // }

  DateTime dt = rtc_clock.now();
  // Serial.printf("    DateTime: %s. Buffering MeteoData.\n", dt.timestamp().c_str());

  // MeteoData &data = meteoDataBuffer[num_meteo_data_collected];
  // data.datetime = dt;
  // data.compute_statistics(fineDataBuffer, NUM_FINE_VALUES, num_fine_data_collected);

  // if(VERBOSE >= 1)
  // {
  //   char msg[400];
  //   Serial.printf("    %s\n", data.print(msg));
  //   // data.dataToCsvLine(msg);
  //   // Serial.println(msg);
  // }

  // write data into buffer
  // num_meteo_data_collected++;

  // start over from the beginning of buffer
  // num_fine_data_collected = 0;
}

// write the meteo data buffer to CSV
void meteo_data_write()
{
  // Fill the base class pointer array with addresses of derived class objects
  // Logger::printf(Logger::INFO, "meteo_data_write: %d collected", num_meteo_data_collected);
  // DataBase* dbPtr[num_meteo_data_collected];
  // for (int i = 0; i < num_meteo_data_collected; i++) {
  //     dbPtr[i] = &meteoDataBuffer[i];
  // }

  // Logger::print("meteo_data_write - CSVHandler::appendData");
  // CSVHandler::appendData(data_meteo_filename, dbPtr, num_meteo_data_collected);
  // // start over from the beginning of buffer
  // num_meteo_data_collected = 0;
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
    if(VERBOSE >= 1)
    {
      // Serial.printf("DateTime: %s. Writing PR2Data[a%d].\n", dt.timestamp().c_str(), pr2_addresses[iss]);
      char msg[400];
      Serial.printf("PR2[a%d]: %s\n",pr2_addresses[iss], pr2_readers[iss].data.print(msg, sizeof(msg)));
    }

    Logger::print("collect_and_write_PR2 - CSVHandler::appendData");
    CSVHandler::appendData(data_pr2_filenames[iss], &(pr2_readers[iss].data));

    pr2_readers[iss].Reset();
    iss++;
    if(iss == n_pr2_sensors)
    {
      iss = 0;
      pr2_all_finished = true;
      digitalWrite(PR2_POWER_PIN, LOW);  // turn off power for PR2
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
  String summary = "";

  Serial.println("Starting HLAVO station setup.");

  // necessary for I2C
  // for version over 3.5 need to turn uSUP ON
  Serial.print("set power pin: "); Serial.println(PIN_ON);
  pinMode(PIN_ON, OUTPUT);      // Set EN pin for uSUP stabilisator as output
  digitalWrite(PIN_ON, HIGH);   // Turn on the uSUP power
  summary += " - POWER PIN " +  String(PIN_ON) + " on\n";


  // I2C setup
  if(Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN))
  {
    Serial.println("TwoWire (I2C) is ready to use.");
    summary += " - I2C [SDA " + String(I2C_SDA_PIN) + " SCL " + String(I2C_SCL_PIN) + "] ready\n";
  }
  else
  {
    Serial.println("TwoWire (I2C) initialization failed.");
    Serial.println(setup_interrupt);
    while(1){delay(1000);}
  }

  // clock setup
  if(rtc_clock.begin())
  {
    Serial.println("RTC is ready to use.");
    summary += " - RTC ready\n";
  }
  else
  {
    Serial.println("RTC initialization failed.");
    Serial.println(setup_interrupt);
    while(1){delay(1000);}
  }
  DateTime dt = rtc_clock.now();

// SD card setup
  pinMode(SD_CS_PIN, OUTPUT);
  // SD Card Initialization
  if (SD.begin()){
      Serial.println("SD card is ready to use.");
      summary += " - SD card [pin " + String(SD_CS_PIN) + "] ready \n";
  }
  else{
      Serial.println("SD card initialization failed.");
      Serial.println(setup_interrupt);
      while(1){delay(1000);}
  }
  Logger::setup_log(rtc_clock, "logs");
  Serial.println("Log set up.");
  Logger::print("Log set up.");

  // BH1750 - Light
  if(lightMeter.begin())
  {
    summary += " - BH1750 ready\n";
  }
  else
  {
    summary += " - BH1750 FAILED\n";
    Logger::print("BH1750 (light) not found.", Logger::WARN);
  }

  // BME280 - temperature, pressure, humidity
  tempSensor.setI2CAddress(tempSensor_I2C); // set I2C address, default 0x77
  if(tempSensor.beginI2C())
  {
    tempSensor.setFilter(0); //0 to 4 is valid. Filter coefficient. See 3.4.4
    tempSensor.setStandbyTime(0); //0 to 7 valid. Time between readings. See table 27.
    tempSensor.setTempOverSample(1); //0 to 16 are valid. 0 disables temp sensing. See table 24.
    tempSensor.setPressureOverSample(1); //0 to 16 are valid. 0 disables pressure sensing. See table 23.
    tempSensor.setHumidityOverSample(1); //0 to 16 are valid. 0 disables humidity sensing. See table 19.
    tempSensor.setMode(MODE_FORCED); //MODE_SLEEP, MODE_FORCED, MODE_NORMAL is valid.
    summary += " - BME280 ready\n";
  }
  else
  {
    summary += " - BME280 FAILED\n";
    Logger::print("BME280 not found.", Logger::WARN);
  }

  gpio_install_isr_service( ESP_INTR_FLAG_IRAM);
  
  // pumps pins, reset
  pinMode(PUMP_OUT_PIN, OUTPUT);
  digitalWrite(PR2_POWER_PIN, LOW);
  pinMode(PUMP_IN_PIN, OUTPUT);
  digitalWrite(PR2_POWER_PIN, LOW);

  // PR2
  pinMode(PR2_POWER_PIN, OUTPUT);
  digitalWrite(PR2_POWER_PIN, HIGH);  // turn on power for PR2
  timer_PR2_power.reset();

  delay(1000);
  Serial.println("Opening SDI-12 for PR2...");
  pr2.begin();

  delay(1000);  // allow things to settle
  uint8_t nbytes = 0;
  Serial.println(pr2.requestAndReadData("?I!", &nbytes));  // Command to get sensor info


  // Data files setup
  char csvLine[400];
  const char* flow_dir="flow";
  CSVHandler::createFile(data_flow_filename,
                         ColumnFlowData::headerToCsvLine(csvLine, max_csvline_length),
                         dt, flow_dir);
  for(int i=0; i<n_pr2_sensors; i++){
    char pr2_dir[20];
    sprintf(pr2_dir, "pr2_sensor_%d", i);
    CSVHandler::createFile(data_pr2_filenames[i],
                              PR2Data::headerToCsvLine(csvLine, max_csvline_length),
                              dt, pr2_dir);
  }

  for(int i=0; i<2; i++)
    rain_regimes[i].compute_trigger();

  print_setup_summary(summary);
  delay(5000);

  // synchronize timers after setup
  timer_L2.reset(true);
  timer_L1.reset(true);
  timer_L4.reset(false);
}

void print_setup_summary(String summary)
{
  summary = "\nSETUP SUMMARY:\n" + summary;
  summary = "\n=======================================================================\n" + summary + "\n";
  summary += F("INO file: " __FILE__ " " __DATE__ " " __TIME__ "\n\n");
  summary += "=======================================================================";

  Logger::print(summary);
  Logger::print("HLAVO station is running");
}


void scan_I2C()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);
}


void measureBME280()
{
  // Serial.print("Humidity: ");
  // Serial.print(tempSensor.readFloatHumidity(), 0);
  // Serial.print(" Pressure: ");
  // Serial.print(tempSensor.readFloatPressure(), 0);
  // Serial.print(" Temp: ");
  // Serial.print(tempSensor.readTempC(), 2);
  // Serial.println();
  
  BME280_SensorMeasurements measurements;
  tempSensor.readAllMeasurements(&measurements); // tempScale = 0 => Celsius

  Serial.printf("Humidity: %.0f, Pressure: %.0f, Temperature: %.2f\n", measurements.humidity, measurements.pressure, measurements.temperature);
}



/*********************************************** LOOP ***********************************************/ 
void loop() {
  
  // stop pump out
  if(!pump_out_finished && timer_outflow.after())
  {
    digitalWrite(PUMP_OUT_PIN, LOW);  // turn off power for PR2
    pump_out_finished = true;
  }

  // read water height
  if(timer_L1())
  {
    Serial.println("        -------------------------- L1 TICK --------------------------");
    read_water_height();
  }

  // scan_I2C();

  // read values from PR2 sensors when reading not finished yet
  // and write to a file when last values received
  if(!pr2_all_finished && timer_PR2_power.after())
    collect_and_write_PR2();

  // request reading from PR2 sensors
  // and write Meteo Data buffer to a file
  if(timer_L2())
  {
    Serial.println("-------------------------- L2 TICK --------------------------");
    Logger::print("L2 TICK");
    
    Serial.println(rtc_clock.now().timestamp().c_str());

    float light_lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.println(light_lux);

    measureBME280();


    meteo_data_write();

    pr2_all_finished = false;
    // Serial.println("PR2 power on.");
    digitalWrite(PR2_POWER_PIN, HIGH);  // turn on power for PR2
    timer_PR2_power.reset();

    #ifdef TEST
      // TEST read data from CSV
      // CSVHandler::printFile(data_meteo_filename);
      // for(int i=0; i<n_pr2_sensors; i++){
      //   CSVHandler::printFile(data_pr2_filenames[i]);
      // }
    #endif
  }

  // if(timer_L4())
  // {
  //   Serial.println("-------------------------- L4 TICK --------------------------");
  //   Logger::print("L4 TICK - Reboot");
  //   Serial.printf("\nReboot...\n\n");
  //   delay(250);
  //   ESP.restart();
  // }
}