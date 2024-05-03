#include "meteo_data.h"
#include <Every.h>

#define PIN_ON 47 // napajeni !!!

/** TIMERS*/
Every timer_fine(1000); // milliseconds

/*********************************************** SD CARD ***********************************************/
// SD card IO
#include "SD.h"
// file handling
#include "file_info.h"
// SD card pin
#define SD_CS_PIN 10
#define data_meteo_filename "/meteo.txt"


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
// TODO: 60 s
#define WEATHER_PERIOD 10  // data refresh in seconds
#define WINDVANE_PIN A0   // A0 := 1
#define ANEMOMETER_PIN 5
#define RAINGAUGE_PIN 6  // 10 kOhm / 10pF
#include "weather_station.h"
WeatherStation weather(WINDVANE_PIN, ANEMOMETER_PIN, RAINGAUGE_PIN, WEATHER_PERIOD);

// interuption
// ICACHE_RAM_ATTR replaced by IRAM_ATTR (esp and arduino>3.0.0)
void IRAM_ATTR intAnemometer() { weather.intAnemometer(); }
void IRAM_ATTR intRaingauge() { weather.intRaingauge(); }
void IRAM_ATTR intPeriod() { weather.intTimer(); }


/****************************************** DATA COLLECTION ******************************************/
float fineValuesBuffer[NUM_FINE_VALUES][NUM_FINE_VALUES_TO_COLLECT];
int num_fine_values_collected = 0;

// collects data at fine interval
void timer_fine_work()
{
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp);
  // float light_lux = lightMeter.readLightLevel();
  float light_lux = 1.11;

  DateTime dt = rtc_clock.now();

  int i = num_fine_values_collected;
  fineValuesBuffer[0][i] = humidity.temperature;
  fineValuesBuffer[1][i] = temp.temperature;
  fineValuesBuffer[2][i] = light_lux;
  fineValuesBuffer[3][i] = adc.readVoltage() * DeviderRatio;

  Serial.printf("%d:   %.2f,  %.2f,  %.0f,  %.3f\n", num_fine_values_collected,
    fineValuesBuffer[0][i], fineValuesBuffer[1][i], fineValuesBuffer[2][i], fineValuesBuffer[3][i]);

  num_fine_values_collected++;
}

// every NUM_FINE_VALUES_TO_COLLECT save the data to buffer
void timer_fine_buffer_release()
{
  DateTime dt = rtc_clock.now();
  Serial.printf("DateTime: %s\n", dt.timestamp().c_str());

  MeteoData data;
  data.datetime = dt;
  data.compute_statistics(fineValuesBuffer, NUM_FINE_VALUES, num_fine_values_collected);
  // start over from the beginning of buffer
  num_fine_values_collected = 0;

  if (weather.gotData())
  {
    data.wind_direction = weather.getDirection();
    data.wind_speed_ticks = weather.getSpeedTicks();
    data.raingauge_ticks = weather.getRainTicks();
  }

  char csvLine[150];
  FileInfo datafile(SD, data_meteo_filename);
  datafile.append(data.dataToCsvLine(csvLine));
}

/*********************************************** SETUP ***********************************************/ 
void setup() {
  Serial.begin(115200);
  while (!Serial)
  {
      ; // cekani na Serial port
  }

  Serial.println("HLAVO project starts.");

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

  char csvLine[150];
  FileInfo datafile(SD, data_meteo_filename);
  datafile.remove();
  // datafile.read();
  if(!datafile.exists())
    datafile.write(MeteoData::headerToCsvLine(csvLine));

  Serial.println("setup completed.");
  Serial.println(F("Start loop " __FILE__ " " __DATE__ " " __TIME__));
  Serial.println("--------------------------");
}


/*********************************************** LOOP ***********************************************/ 
void loop() {
  
  weather.update();

  // read value to buffer at fine time scale
  if(timer_fine())
  {
    // Serial.printf("Fine tick\n");
    timer_fine_work();
  }

	if (weather.gotData()) {
    // Serial.printf("Weather tick\n");

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

    // char csvLine[150];
    // FileInfo datafile(SD, data_meteo_filename);
    // datafile.append(data.dataToCsvLine(csvLine));

    // datafile.read();

    timer_fine_buffer_release();

    // TEST read data from CSV
    datafile.read();

    weather.resetGotData();
    Serial.println("--------------------------");
  }
}