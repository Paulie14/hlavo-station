#include "WeatherMeters.h"

#define PIN_ON 47 // napajeni !!!

const int windvane_pin = A0; 
const int anemometer_pin = 5; 
const int raingauge_pin = 6; // 10 kOhm / 10pF

volatile bool got_data = false;

hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
volatile bool do_update = false;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

WeatherMeters <4> meters(windvane_pin, 8);  // filter last 4 directions, refresh data every 8 sec

void ICACHE_RAM_ATTR intAnemometer() {
	meters.intAnemometer();
}

void ICACHE_RAM_ATTR intRaingauge() {
	meters.intRaingauge();
}

void IRAM_ATTR onTimer() {
	xSemaphoreGiveFromISR(timerSemaphore, NULL);
	do_update = true;
}

void readDone(void) {
	got_data = true;
}

 
void setup() {
  Serial.begin(115200);

  Serial.println("HLAVO project starts.");

  // for version over 3.5 need to turn uSUP ON
  pinMode(PIN_ON, OUTPUT);      // Set EN pin for uSUP stabilisator as output
  digitalWrite(PIN_ON, HIGH);   // Turn on the uSUP power

  Serial.println("HLAVO project starts.");

  attachInterrupt(digitalPinToInterrupt(anemometer_pin), intAnemometer, FALLING);
	attachInterrupt(digitalPinToInterrupt(raingauge_pin), intRaingauge, FALLING);

	meters.attach(readDone);

	timerSemaphore = xSemaphoreCreateBinary();
	timer = timerBegin(0, 80, true);
	timerAttachInterrupt(timer, &onTimer, true);
	timerAlarmWrite(timer, 1000000, true);
	timerAlarmEnable(timer);

	meters.reset();  // in case we got already some interrupts
}
 
void loop() {
  if(do_update){
		meters.timer();
		do_update = false;
	}

	if (got_data) {
		got_data = false;

        Serial.print("Směr větru: "); Serial.print(meters.getDir()); Serial.println(" deg");

        Serial.print("Rychlost větru TICK: "); Serial.println(meters.getSpeedTicks());
        Serial.print("Srážky TICK: "); Serial.println(meters.getRainTicks());

        Serial.print("Rychlost větru: "); Serial.print(meters.getSpeed()); Serial.println(" km/h"); 
        Serial.print("Srážky: "); Serial.print(meters.getRain()); Serial.println(" mm");

        Serial.println("--------------------------");
   }
 
  delay(1);
}