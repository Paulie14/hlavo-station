
/*********************************************** COMMON ***********************************************/
#include <Every.h>
#include <Logger.h>

#define PIN_ON 47 // napajeni !!!

const char* setup_interrupt = "SETUP INTERRUPTED";

/************************************************ RUN ************************************************/
// Switch between testing/setup and long term run.
#define TEST

#ifdef TEST
    /** TIMERS */
    // times in milliseconds, L*... timing level
    Every timer_L1(2000);      // fine timer - humidity, temperature, meteo, ...
    // L2 - hardware timer with L2_WEATHER_PERIOD in seconds
    #define L2_WEATHER_PERIOD 10
    Every timer_L3(60*1000); // coarse timer - PR2 - 40 s
    Every timer_L4(10*60*1000);  // watchdog timer - 10 min
    #define VERBOSE 1
#else
    /** TIMERS */
    // times in milliseconds, L*... timing level
    Every timer_L1(1000);         // fine timer - humidity, temperature, ...
    // L2 - hardware timer with L2_WEATHER_PERIOD in seconds
    #define L2_WEATHER_PERIOD 60
    Every timer_L3(900*1000);     // coarse timer - PR2 - 15 min
    Every timer_L4(24*3600*1000); // watchdog timer - 24 h
    #define VERBOSE 1
#endif


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
char data_pr2_filenames[n_pr2_sensors][100] = {"pr2_a3.csv"};

uint8_t iss = 0;  // current sensor reading
bool pr2_all_finished = false;

Timer timer_PR2_power(2000, false);

/****************************************** DATA COLLECTION ******************************************/
// L1 timer data buffer
// float fineDataBuffer[NUM_FINE_VALUES][FINE_DATA_BUFSIZE];
int num_fine_data_collected = 0;

// collect meteo data at fine interval into a fine buffer of floats
void fine_data_collect()
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

  num_fine_data_collected++;
}

// compute statistics over the fine meteo data
// and save the meteo data into buffer of MeteoData
void meteo_data_collect()
{
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
  num_fine_data_collected = 0;
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
    summary += " - BME280 ready\n";
  }
  else
  {
    summary += " - BME280 FAILED\n";
    Logger::print("BME280 not found.", Logger::WARN);
  }


  // tempSensor.setFilter(0); //0 to 4 is valid. Filter coefficient. See 3.4.4
  // tempSensor.setStandbyTime(0); //0 to 7 valid. Time between readings. See table 27.
  // tempSensor.setTempOverSample(1); //0 to 16 are valid. 0 disables temp sensing. See table 24.
  // tempSensor.setPressureOverSample(1); //0 to 16 are valid. 0 disables pressure sensing. See table 23.
  // tempSensor.setHumidityOverSample(1); //0 to 16 are valid. 0 disables humidity sensing. See table 19.
  // tempSensor.setMode(MODE_FORCED); //MODE_SLEEP, MODE_FORCED, MODE_NORMAL is valid.

  // // PR2
  // gpio_install_isr_service( ESP_INTR_FLAG_IRAM);
  // pinMode(PR2_POWER_PIN, OUTPUT);
  // digitalWrite(PR2_POWER_PIN, HIGH);  // turn on power for PR2
  // timer_PR2_power.reset();

  // delay(1000);
  // Serial.println("Opening SDI-12 for PR2...");
  // pr2.begin();

  // delay(1000);  // allow things to settle
  // uint8_t nbytes = 0;
  // pr2.requestAndReadData("?I!", &nbytes);  // Command to get sensor info


  // // Data files setup
  // char csvLine[400];
  // const char* meteo_dir="meteo";
  // CSVHandler::createFile(data_meteo_filename,
  //                           MeteoData::headerToCsvLine(csvLine),
  //                           dt, meteo_dir);
  // for(int i=0; i<n_pr2_sensors; i++){
  //   char pr2_dir[20];
  //   sprintf(pr2_dir, "pr2_sensor_%d", i);
  //   CSVHandler::createFile(data_pr2_filenames[i],
  //                             PR2Data::headerToCsvLine(csvLine),
  //                             dt, pr2_dir);
  // }

  print_setup_summary(summary);
  delay(5000);

  // synchronize timers after setup
  timer_L3.reset(true);
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
  BME280_SensorMeasurements measurements;
  tempSensor.readAllMeasurements(&measurements); // tempScale = 0 => Celsius

  Serial.printf("Humidity: %.0f, Pressure: %.0f, Temperature: %.2f\n", measurements.humidity, measurements.pressure, measurements.temperature);
}



/*********************************************** LOOP ***********************************************/ 
void loop() {
  
  // read values to buffer at fine time scale [fine Meteo Data]
  if(timer_L1())
  {
    Serial.println("        -------------------------- L1 TICK --------------------------");
    // fine_data_collect();


    Serial.println(rtc_clock.now().timestamp().c_str());

    float light_lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.println(light_lux);

    // Serial.print("Humidity: ");
    // Serial.print(tempSensor.readFloatHumidity(), 0);
    // Serial.print(" Pressure: ");
    // Serial.print(tempSensor.readFloatPressure(), 0);
    // Serial.print(" Temp: ");
    // Serial.print(tempSensor.readTempC(), 2);
    // Serial.println();

    measureBME280();

  }

  // scan_I2C();

  // read values to buffer at fine time scale [averaged Meteo Data]
	// if (weather.gotData()) {
  //   Serial.println("    **************************************** L2 TICK ****************************************");

  //   meteo_data_collect();
  //   weather.resetGotData();
  //   Serial.println("    **************************************** ******* ****************************************");
  // }

  // // read values from PR2 sensors when reading not finished yet
  // // and write to a file when last values received
  // if(!pr2_all_finished && timer_PR2_power.after())
  //   collect_and_write_PR2();

  // // request reading from PR2 sensors
  // // and write Meteo Data buffer to a file
  // if(timer_L3())
  // {
  //   Serial.println("-------------------------- L3 TICK --------------------------");
  //   Logger::print("L3 TICK");
  //   meteo_data_write();

  //   pr2_all_finished = false;
  //   // Serial.println("PR2 power on.");
  //   digitalWrite(PR2_POWER_PIN, HIGH);  // turn on power for PR2
  //   timer_PR2_power.reset();

  //   #ifdef TEST
  //     // TEST read data from CSV
  //     // CSVHandler::printFile(data_meteo_filename);
  //     // for(int i=0; i<n_pr2_sensors; i++){
  //     //   CSVHandler::printFile(data_pr2_filenames[i]);
  //     // }
  //   #endif
  // }

  // if(timer_L4())
  // {
  //   Serial.println("-------------------------- L4 TICK --------------------------");
  //   Logger::print("L4 TICK - Reboot");
  //   Serial.printf("\nReboot...\n\n");
  //   delay(250);
  //   ESP.restart();
  // }
}