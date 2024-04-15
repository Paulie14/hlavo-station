// sbernice
#include <Wire.h>
// temperature, humidity
#include "Adafruit_SHT4x.h"
// communication (sbernice)
#include <SPI.h>
// read voltage of ESP32 battery
#include <ESP32AnalogRead.h>
#include "WeatherMeters.h"
// real time
#include <RTClib.h>
// SD card IO
#include <SD.h>