#include "WeatherMeters.h"

#define PIN_ON 47 // napajeni !!!

/*********************************************** SD CARD ***********************************************/
// SD card IO
#include "SD.h"
// file handling
#include "file_io.h"
// SD card pin
#define SD_CS_PIN 10
#define data_meteo_filename "/meteo.txt"

/************************************************* RTC *************************************************/
// definice sbernice i2C pro RTC (real time clock)
#define rtc_SDA 42 // data pin
#define rtc_SCL 2  // clock pin
#include "clock.h"
Clock rtc_clock(rtc_SDA, rtc_SCL);

/****************************************** WHEATHER STATION ******************************************/
#define WEATHER_PERIOD 4  // data refresh in seconds
const int windvane_pin = A0; // A0 := 1
const int anemometer_pin = 5;
const int raingauge_pin = 6; // 10 kOhm / 10pF

volatile bool got_data = false;

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
volatile bool do_update = false;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

WeatherMeters <4> meters(windvane_pin, WEATHER_PERIOD);  // filter last 4 directions, refresh data every * sec

// ICACHE_RAM_ATTR replaced by IRAM_ATTR (esp and arduino>3.0.0)
void IRAM_ATTR intAnemometer() {
	meters.intAnemometer();
}

void IRAM_ATTR intRaingauge() {
	meters.intRaingauge();
}

void IRAM_ATTR onTimer() {
	xSemaphoreGiveFromISR(timerSemaphore, NULL);
	do_update = true;
}

void readDone(void) {
	got_data = true;
}

struct MeteoData{

  DateTime datetime;

  // weather station
  float wind_direction;
  unsigned int wind_speed_ticks;
  unsigned int raingauge_ticks;

  // RTC
  float humidity;
  float temperature;

  // battery - ESP32 analog read
  float battery_voltage;
};


// Function to convert MeteoData struct to CSV string with a custom delimiter
char* meteoDataToCSV(struct MeteoData data, char* csvLine) {

  char delimiter = ';';

  const char * datetime = data.datetime.timestamp().c_str();
  // Format datetime in "YYYY-MM-DD HH:MM:SS" format
  // sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
  //         data.datetime.year(), data.datetime.month(), data.datetime.day(),
  //         data.datetime.hour(), data.datetime.minute(), data.datetime.second());
  sprintf(csvLine, "%s%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
          datetime, delimiter,
          data.wind_direction, delimiter,
          data.wind_speed_ticks, delimiter,
          data.raingauge_ticks, delimiter,
          data.humidity, delimiter,
          data.temperature, delimiter,
          data.battery_voltage);
  return csvLine;
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

  rtc_clock.begin();

  // weather station
  pinMode(windvane_pin, ANALOG);
  pinMode(raingauge_pin, INPUT_PULLUP);  // Set GPIO as input with pull-up (like adding 10k resitor)
  pinMode(anemometer_pin, INPUT_PULLUP);  // Set GPIO as input with pull-up (like adding 10k resitor)
  attachInterrupt(digitalPinToInterrupt(anemometer_pin), intAnemometer, CHANGE);
	attachInterrupt(digitalPinToInterrupt(raingauge_pin), intRaingauge, CHANGE);

	meters.attach(readDone);

	timerSemaphore = xSemaphoreCreateBinary();
	timer = timerBegin(0, 80, true);
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, 1000000, true);
	timerAlarmEnable(timer);

	meters.reset();  // in case we got already some interrupts

  // SD card
  pinMode(SD_CS_PIN, OUTPUT);
  // SD Card Initialization
  if (SD.begin()){
      Serial.println("SD card is ready to use.");
  }
  else{
      Serial.println("SD card initialization failed");
      return;
  }

  Serial.println("setup completed.");
  Serial.println("--------------------------");
}


/*********************************************** LOOP ***********************************************/ 
void loop() {
  if(do_update){
		meters.timer();
		do_update = false;
	}

	if (got_data) {
		got_data = false;

      MeteoData data;
      data.wind_direction = meters.getDir();
      data.wind_speed_ticks = meters.getSpeedTicks();
      data.raingauge_ticks = meters.getRainTicks();

      Serial.printf("Wind direc adc:  %d\n", meters.getDirAdcValue());
      Serial.printf("Wind direc deg:  %f\n", data.wind_direction);
      Serial.printf("Wind speed TICK: %d\n", data.wind_speed_ticks);
      Serial.printf("Rain gauge TICK: %d\n", data.raingauge_ticks);

      char csvLine[150];
      meteoDataToCSV(data, csvLine);
      FileIO datafile(SD, data_meteo_filename);
      datafile.append(csvLine);

      Serial.println("--------------------------");
   }

  // delay(1);
}