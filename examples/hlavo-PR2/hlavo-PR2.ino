/**
 */

#include <esp_intr_alloc.h>
#include "pr2_sensor.h"

#define SERIAL_BAUD 115200 /*!< The baud rate for the output serial port */
#define PR2_DATA_PIN 4         /*!< The pin of the SDI-12 data bus */
#define POWER_PIN 47       /*!< The sensor power pin (or -1 if not switching power) */

/** Define the SDI-12 bus */
PR2_sensor pr2(PR2_DATA_PIN, 3);

void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial)
    ;

  gpio_install_isr_service( ESP_INTR_FLAG_IRAM);

  Serial.println("Opening SDI-12 bus...");
  pr2.begin();
  delay(500);  // allow things to settle

  // Power the sensors;
  if (POWER_PIN > 0) {
    Serial.println("Powering up sensors...");
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    delay(500);
  }

  Serial.flush();
}

bool human_print = true;
void print_values(String field_name, float* values, uint8_t n_values)
{
  Serial.printf("%-25s", (field_name + ':').c_str());
  for(int i=0; i<n_values; i++)
    Serial.printf("%.4f  ", values[i]);
  Serial.println();
}

void loop() {

  String sensorResponse = "";

  delay(300);

  String si = pr2.requestAndReadData("?I!", false);  // Command to get sensor info
  // Serial.println(si);

  delay(300);

  // sensorResponse = pr2.measureConcurrent("C", 0);
  // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("permitivity:    ");
  // Serial.println(sensorResponse);

  // sensorResponse = pr2.measureConcurrent("C1",0);
  // // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("soil moisture:  ");
  // Serial.println(sensorResponse);

  // sensorResponse = pr2.measureConcurrent("C8",0);
  // // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("millivolts:     ");
  // Serial.println(sensorResponse);

  // sensorResponse = pr2.measureConcurrent("C9",0);
  // // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("raw ADC:        ");
  // Serial.println(sensorResponse);

  float values[10];
  uint8_t n_values = 0;

  sensorResponse = pr2.measureConcurrent("C", 0, values, &n_values);
  print_values("permitivity", values, n_values);

  sensorResponse = pr2.measureConcurrent("C1", 0, values, &n_values);
  print_values("soil moisture mineral", values, n_values);

  sensorResponse = pr2.measureConcurrent("C2", 0, values, &n_values);
  print_values("soil moisture organic", values, n_values);

  sensorResponse = pr2.measureConcurrent("C3", 0, values, &n_values);
  print_values("soil moisture mineral (%)", values, n_values);

  sensorResponse = pr2.measureConcurrent("C4", 0, values, &n_values);
  print_values("soil moisture mineral (%)", values, n_values);

  sensorResponse = pr2.measureConcurrent("C7", 0, values, &n_values);
  print_values("millivolts", values, n_values);

  sensorResponse = pr2.measureConcurrent("C8", 0, values, &n_values);
  print_values("millivolts uncalibrated", values, n_values);

  sensorResponse = pr2.measureConcurrent("C9", 0, values, &n_values);
  print_values("raw ADC", values, n_values);





  // sensorResponse = requestAndReadData("?I!", true);  // Command to get sensor info
  // //Serial.println(sensorResponse);

  // sensorResponse = measureString("C",0);
  // //sensorResponse = sensorResponse.substring(3); // first 3 characters are <address><CR><LF> => remove
  // // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // Serial.print("permitivity:    "); Serial.println(sensorResponse);

  // sensorResponse = measureString("C1",0);
  // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("soil moisture:  ");
  // Serial.println(sensorResponse);

  // sensorResponse = measureString("C8",0);
  // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("millivolts:     ");
  // Serial.println(sensorResponse);

  // sensorResponse = measureString("C9",0);
  // sensorResponse = sensorResponse.substring(1); // first 1 characters is <address> => remove
  // if(human_print) Serial.print("raw ADC:        ");
  // Serial.println(sensorResponse);




  // //Info
  // Serial.printf("Command %s:", myCommand);
  // mySDI12.sendCommand(myCommand);
  // delay(300);                    // wait a while for a response
  // while (mySDI12.available()) {  // write the response to the screen
  //   Serial.write(mySDI12.read());
  // }

  // // const char* cmd1 = "?!";
  // // const char* cmd2 = "0M!";
  // // const char* cmd3 = "0D0!";

  // // Address
  // Serial.printf("Command %s:", myCommand1);
  // mySDI12.sendCommand(myCommand1);
  // // mySDI12.sendCommand(cmd1);
  // delay(300);                    // wait a while for a response
  // // Serial.print("Address: ");
  // String sensorResponse="";
  // while (mySDI12.available()) {  // write the response to the screen
  //   char c = mySDI12.read();
  //   Serial.print(c, HEX); Serial.print(" ");
  //   if (c != -1) {              // Check if the character is valid
  //     sensorResponse += c;      // Append the character to the response string
  //     #if DEBUG
  //       Serial.print(c, HEX); Serial.print(" ");
  //     #endif
  //   }
  //   delay(20);  // otherwise it would leave some chars to next message...
  // //  Serial.write(mySDI12.read());
  //   // char buf[25];
  //   // Serial.printf("%s-", itoa(mySDI12.read(), buf, 10));
  // }
  // Serial.print(sensorResponse);
  // Serial.println();
  // delay(500); 

  // // M7
  // // Serial.printf("Command %s:", "0M7!");
  // // mySDI12.sendCommand("0M7!");
  // // // mySDI12.sendCommand(cmd1);
  // // delay(300);                    // wait a while for a response
  // // // Serial.print("Address: ");
  // // while (mySDI12.available()) {  // write the response to the screen
  // //  Serial.write(mySDI12.read());
  // //   // char buf[25];
  // //   // Serial.printf("%s-", itoa(mySDI12.read(), buf, 10));
  // // }
  // // Serial.println();
  // // delay(500); 


  // // Measure
  //   String cmd = "0C!";
  //   // String cmd = "0C!";
  //   Serial.printf("Command %s:", cmd);
  //   mySDI12.sendCommand(cmd);
  //   delay(300);                    // wait a while for a response
  //   while (mySDI12.available()) {  // build response string
  //       char c = mySDI12.read();
  //       Serial.print(c, HEX); Serial.print(" ");
  //       if ((c != '\n') && (c != '\r'))
  //       {
  //         sdiResponse += c;
  //       }
  //       delay(20);  // 1 character ~ 7.5ms
  //   }
  //   if (sdiResponse.length() > 1)
  //       Serial.println(sdiResponse);  // write the response to the screen
  //   mySDI12.clearBuffer();
  //   sdiResponse = "";  // clear the response string

  //   delay(2000); 

  //   // DATA
  //   Serial.print("Command 0D0!: ");
  //   mySDI12.sendCommand(myCommand3);
  //   // mySDI12.sendCommand(cmd3);
  //   delay(3000);                    // wait a while for a response
    
  //   while (mySDI12.available()) {  // build response string
  //       char c = mySDI12.read();
  //       Serial.print(c, HEX); Serial.print(" ");
  //   // if (c != -1) {              // Check if the character is valid
  //   //   sensorResponse += c;      // Append the character to the response string
  //   //   #if DEBUG
  //   //     Serial.print(c, HEX); Serial.print(" ");
  //   //   #endif
  //   // }
  //   // delay(20);  // otherwise it would leave some chars to next message...

  //       if ((c != '\n') && (c != '\r'))
  //       {
  //         sdiResponse += c;
  //         // char buf[25];
  //         // Serial.printf("%s-", itoa(mySDI12.read(), buf, 10));
  //       }
  //       delay(20);  // 1 character ~ 7.5ms
  //   }
  //   if (sdiResponse.length() > 1)
  //       Serial.println(sdiResponse);  // write the response to the screen
  //   // Serial.println();
  //   mySDI12.clearBuffer();
  //   sdiResponse = "";  // clear the response string

















//   mySDI12.sendCommand(myCommand2);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0M!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
  
//   delay(1000); 

//   mySDI12.sendCommand(myCommand3);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D0!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(2000); 

//   mySDI12.sendCommand(myCommand4);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D1!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//    delay(500); 







//   mySDI12.sendCommand(myCommand5);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0M1!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(2000); 

//   mySDI12.sendCommand(myCommand6);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D0!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500); 

//   mySDI12.sendCommand(myCommand7);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D1!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500); 

//   mySDI12.sendCommand(myCommand8);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0M8!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(2000); 

//   mySDI12.sendCommand(myCommand9);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D0!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500); 

//   mySDI12.sendCommand(myCommand10);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D1!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500);
// mySDI12.sendCommand(myCommand11);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command Raw ADC: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(2000); 

//   mySDI12.sendCommand(myCommand12);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D0!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500); 

//   mySDI12.sendCommand(myCommand13);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command 0D1!: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500);
//   mySDI12.sendCommand(myCommand14);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command Permitivity: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500); 

//   mySDI12.sendCommand(myCommand15);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command Theta: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
//   delay(500);

//   mySDI12.sendCommand(myCommand16);
//   delay(300);                    // wait a while for a response
//   Serial.print("Command Perc Volumetric: ");
//   while (mySDI12.available()) {  // write the response to the screen
//   Serial.write(mySDI12.read());
//   }
  delay(2000);  // print again in three seconds
}