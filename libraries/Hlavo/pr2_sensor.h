#include <SDI12.h>

class PR2_sensor
{
  private:
    const uint16_t _meaure_delay = 2000; // waiting between measure request and data request
    uint8_t _verbose = 0;
    int8_t _dataPin;
    SDI12 _SDI12;
  public:
    PR2_sensor(int8_t dataPin, uint8_t verbose=0)
    : _SDI12(_dataPin), _verbose(verbose)
    {}

    void begin()
    {
      _SDI12.begin();
    }
    

    String requestAndReadData(String command, bool trim = false) {
      _SDI12.sendCommand(command); // Send the SDI-12 command
      delay(300);                   // Wait for response to be ready

      String sensorResponse = "";
      //Read the response from the sensor
      while (_SDI12.available()) { // Check if there is data available to read
        char c = _SDI12.read();    // Read a single character
        if (c != -1) {              // Check if the character is valid
          sensorResponse += c;      // Append the character to the response string
          if(_verbose > 1)
            Serial.print(c, HEX); Serial.print(" ");
        }
        delay(20);  // otherwise it would leave some chars to next message...
      }
      if(_verbose > 1)
        Serial.println("");

    // >> 30 31 33 44 65 6C 74 61 2D 54 20 50 52 32 53 44 49 31 2E 31 50 52 32 2F 36 2D 30 34 35 30 36 30 D A command ?I!: 013Delta-T PR2SDI1.1PR2/6-045060
    // >> 30 30 30 32 36 D A command 0M!: 00026
    // >> 30 D A 30 2B 30 2E 39 38 32 2B 30 2E 39 38 2B 31 2E 30 30 34 38 2B 30 2E 39 38 35 34 2B 30 2E 39 38 37 35 D command 0D0!: 00+0.982+0.98+1.0048+0.9854+0.9875
    // >> A 30 2B 30 2E 39 38 30 32 D A command 0D1!: 0+0.9802
    // >> DATA: 00+0.982+0.98+1.0048+0.9854+0.98750+0.9802

      // command 0D0! returns <address><CRLF><data><CRLF> => readStringUntil would need to called twice
      // String sensorResponse = mySDI12.readStringUntil('\n');

      // Replace \r and \n with empty strings
      // sensorResponse.replace("\r", "");
      // sensorResponse.replace("\n", "");
      if (trim)
        sensorResponse.trim();  // remove CRLF at the end

      if(_verbose > 0)
      {
        // if printf not available (Arduino)
        // char string_buffer[128]; // Buffer to hold the formatted string
        // snprintf(string_buffer, sizeof(string_buffer), "command %s: %s", command.c_str(), sensorResponse.c_str());
        // Serial.println(string_buffer);
        Serial.printf("command %s: %s", command.c_str(), sensorResponse.c_str());
      }

      return sensorResponse;
    }

    String measureConcurrent(String measure_command, uint8_t address)
    {
        _SDI12.clearBuffer();

        measure_command = String(address) + measure_command + "!";
        String measureResponse = requestAndReadData(measure_command);  // Command to take a measurement
        // Serial.println(measureResponse);

        delay(_meaure_delay);

        // for C commands
        // max message length 75 B
        uint8_t position = 0;
        String data_command = String(address) + "D" + String(position) + "!";
        String sensorResponse = requestAndReadData(data_command);

        // for M commands - multiple D commands necessary
        // max message length 35 B
        // Position in the data command request (multiple calls may be needed to
        // get all values from a sensor).
        // String sensorResponse = "";
        // uint8_t position = 0;
        // const uint8_t max_values = 10; // aD0! ... aD9! range for PR2
        // while (position < max_values) {
        //     // Request data as it should be ready to be read now
        //     String data_command = String(address) + "D" + String(position) + "!";
        //     // Serial.println(data_command);
        //     sensorResponse += requestAndReadData(data_command);

        //     // Increment the position in the data command to get more measurements
        //     // until all values hav been received
        //     position++;
        //     delay(20);
        // }
        return sensorResponse;
    }
};
