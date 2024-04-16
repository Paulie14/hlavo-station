/**
 * @file f_basic_data_request.ino
 * @copyright (c) 2013-2020 Stroud Water Research Center (SWRC)
 *                          and the EnviroDIY Development Team
 *            This example is published under the BSD-3 license.
 * @author Ruben Kertesz <github@emnet.net> or @rinnamon on twitter
 * @date 2/10/2016
 *
 * @brief Example F: Basic Data Request to a Single Sensor
 *
 * This is a very basic (stripped down) example where the user initiates a measurement
 * and receives the results to a terminal window without typing numerous commands into
 * the terminal.
 *
 * Edited by Ruben Kertesz for ISCO Nile 502 2/10/2016
 */

#include <SDI12.h>

#define SERIAL_BAUD 115200 /*!< The baud rate for the output serial port */
// Arduino
// #define DATA_PIN 8         /*!< The pin of the SDI-12 data bus */
// #define POWER_PIN -1       /*!< The sensor power pin (or -1 if not switching power) */
// #define SENSOR_ADDRESS 0

// ESP32-S3
#define DATA_PIN 4         /*!< The pin of the SDI-12 data bus */
#define POWER_PIN 47       /*!< The sensor power pin (or -1 if not switching power) */
#define SENSOR_ADDRESS 0

/** Define the SDI-12 bus */
SDI12 mySDI12(DATA_PIN);

String sdiResponse = "";
String myCommand   = "";

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial)
    ;

    gpio_install_isr_service( ESP_INTR_FLAG_IRAM);

  Serial.println("Opening SDI-12 bus...");
  mySDI12.begin();
  delay(500);  // allow things to settle

  // Power the sensors;
  if (POWER_PIN > 0) {
    Serial.println("Powering up sensors...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(200);
    Serial.println("Powering up sensors...");
  }
}

void loop() {
  // do {  // wait for a response from the serial terminal to do anything
  //   delay(30);
  // } while (!Serial.available());
  // char nogo =
  //   Serial.read();  // simply hit enter in the terminal window or press send and the
  //                   // characters get discarded but now the rest of the loop continues

  // first command to take a measurement
  myCommand = String(SENSOR_ADDRESS) + "I!";
  Serial.println(myCommand);  // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(200);  // wait a while for a response

  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(20);  // 1 character ~ 7.5ms
    }
  }
  if (sdiResponse.length() > 1)
    Serial.println(sdiResponse);  // write the response to the screen
  mySDI12.clearBuffer();
  sdiResponse = "";  // clear the response string

  delay(2000);       // delay between taking reading and requesting data


  // first command to take a measurement
  myCommand = String(SENSOR_ADDRESS) + "MC!";
  Serial.println(myCommand);  // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(200);  // wait a while for a response

  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(20);  // 1 character ~ 7.5ms
    }
  }
  if (sdiResponse.length() > 1)
    Serial.println(sdiResponse);  // write the response to the screen
  mySDI12.clearBuffer();
  sdiResponse = "";  // clear the response string

  delay(2000);       // delay between taking reading and requesting data
  
  // next command to request data from last measurement
  myCommand = String(SENSOR_ADDRESS) + "D0!";
  Serial.println(myCommand);  // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(200);  // wait a while for a response

  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(200);  // 1 character ~ 7.5ms
    }
  }
  if (sdiResponse.length() > 1)
    Serial.println(sdiResponse);  // write the response to the screen
  mySDI12.clearBuffer();
  sdiResponse = "";  // clear the response string


  // next command to request data from last measurement
  myCommand = String(SENSOR_ADDRESS) + "D1!";
  Serial.println(myCommand);  // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(200);  // wait a while for a response

  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(200);  // 1 character ~ 7.5ms
    }
  }
  if (sdiResponse.length() > 1)
    Serial.println(sdiResponse);  // write the response to the screen
  mySDI12.clearBuffer();
  sdiResponse = "";  // clear the response string
  // now go back to top and wait until user hits enter on terminal window
}
