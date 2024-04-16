/**
 */

#define CONFIG_IDF_TARGET_ESP32S2 1 //natvrdo esp32s2 režim
#define ADC_WIDTH_BIT_13 ADC_WIDTH_BIT_12

#include <Arduino.h>
#include <esp32-sdi12.h>

#define SERIAL_BAUD 115200 /*!< The baud rate for the output serial port */
#define SDI12_DATA_PIN 4

// #define POWER_PIN 47       /*!< The sensor power pin (or -1 if not switching power) */



ESP32_SDI12 sdi12(SDI12_DATA_PIN);
float values[10];

void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial)
    ;

    // Power the sensors;
    // if (POWER_PIN > 0) {
    //     Serial.println("Powering up sensors...");
    //     pinMode(POWER_PIN, OUTPUT);
    //     digitalWrite(POWER_PIN, HIGH);
    //     delay(500);//(200);
    // }

    // Initialise SDI-12 pin definition
    sdi12.begin();
}

void loop() {
    // Measure on address 1
    // Float response will be inserted into values buffer
    // ESP32_SDI12::Status res = sdi12.measure(0, values, sizeof(values));
    // if(res != ESP32_SDI12::SDI12_OK){
    //     Serial.printf("Error: %d\n", res);
    // }

    ESP32_SDI12::Sensor * si;
    ESP32_SDI12::Status res = sdi12.sensorInfo(0, si);
    Serial.printf("ESP32_SDI12::Status: %d\n", res);

    // Do something with the data here...

    delay(5000); // Do this measurement every 15 seconds
}
