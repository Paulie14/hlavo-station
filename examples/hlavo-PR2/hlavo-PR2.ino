/**
 */

#include <SDI12.h>
#include <esp_intr_alloc.h>

#define SERIAL_BAUD 115200 /*!< The baud rate for the output serial port */
#define DATA_PIN 4         /*!< The pin of the SDI-12 data bus */
#define POWER_PIN 47       /*!< The sensor power pin (or -1 if not switching power) */

/** Define the SDI-12 bus */
SDI12 mySDI12(DATA_PIN);

/**
  '?' is a wildcard character which asks any and all sensors to respond
  'I' indicates that the command wants information about the sensor
  '!' finishes the command
*/
String myCommand = "?I!";
String myCommand1 = "?!";
String myCommand2 = "0M!";
String myCommand3 = "0D0!";
String myCommand4 = "0D1!";
String myCommand5 = "0M1";
String myCommand6 = "0D0!";
String myCommand7 = "0D1!";
String myCommand8 = "0M8!";
String myCommand9 = "0D0!";
String myCommand10 = "0D1!";
String myCommand11 = "0M9!";
String myCommand12 = "0D0!";
String myCommand13 = "0D1!";
String myCommand14 = "0XWM6U3!";
String myCommand15 = "0XWM6UT!";
String myCommand16 = "0XWM6UV!";

String sdiResponse = "";

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
    delay(500);//(200);
  }

  Serial.flush();
}

void loop() {
  //Info
  Serial.printf("Command %s:", myCommand);
  mySDI12.sendCommand(myCommand);
  delay(300);                    // wait a while for a response
  while (mySDI12.available()) {  // write the response to the screen
    Serial.write(mySDI12.read());
  }

  // const char* cmd1 = "?!";
  // const char* cmd2 = "0M!";
  // const char* cmd3 = "0D0!";

  // Address
  Serial.printf("Command %s:", myCommand1);
  mySDI12.sendCommand(myCommand1);
  // mySDI12.sendCommand(cmd1);
  delay(300);                    // wait a while for a response
  // Serial.print("Address: ");
  while (mySDI12.available()) {  // write the response to the screen
  //  Serial.write(mySDI12.read());
    char buf[25];
    Serial.printf("%s-", itoa(mySDI12.read(), buf, 10));
  }
  Serial.println();
  delay(500); 

  // M7
  Serial.printf("Command %s:", "0M7!");
  mySDI12.sendCommand("0M7!");
  // mySDI12.sendCommand(cmd1);
  delay(300);                    // wait a while for a response
  // Serial.print("Address: ");
  while (mySDI12.available()) {  // write the response to the screen
   Serial.write(mySDI12.read());
    // char buf[25];
    // Serial.printf("%s-", itoa(mySDI12.read(), buf, 10));
  }
  Serial.println();
  delay(500); 


  // Measure
    Serial.printf("Command %s:", myCommand2);
    mySDI12.sendCommand(myCommand2);
    // mySDI12.sendCommand(cmd2);
    delay(300);                    // wait a while for a response
    while (mySDI12.available()) {  // build response string
        char c = mySDI12.read();
        if ((c != '\n') && (c != '\r'))
        {
        sdiResponse += c;
        delay(10);  // 1 character ~ 7.5ms
        }
    }
    if (sdiResponse.length() > 1)
        Serial.println(sdiResponse);  // write the response to the screen
    mySDI12.clearBuffer();
    sdiResponse = "";  // clear the response string

    delay(2000); 

    // DATA
    Serial.print("Command 0D0!: ");
    mySDI12.sendCommand(myCommand3);
    // mySDI12.sendCommand(cmd3);
    delay(3000);                    // wait a while for a response
    
    while (mySDI12.available()) {  // build response string
        char c = mySDI12.read();
        if ((c != '\n') && (c != '\r'))
        {
          sdiResponse += c;
          // char buf[25];
          // Serial.printf("%s-", itoa(mySDI12.read(), buf, 10));
          delay(20);  // 1 character ~ 7.5ms
        }
    }
    if (sdiResponse.length() > 1)
        Serial.println(sdiResponse);  // write the response to the screen
    // Serial.println();
    mySDI12.clearBuffer();
    sdiResponse = "";  // clear the response string


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