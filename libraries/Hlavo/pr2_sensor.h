#include <SDI12.h>

class PR2_sensor
{
  private:
    static const uint16_t _meaure_delay = 2200; // waiting between measure request and data request
    static const uint8_t n_bytes_per_val = 7;
    static const uint8_t max_msg_length = 100;

    uint8_t _verbose = 0;
    int8_t _dataPin;
    SDI12 _SDI12;

    char msg_buf[max_msg_length];

  public:
    PR2_sensor(int8_t dataPin, uint8_t verbose=0)
    : _dataPin(dataPin), _SDI12(dataPin), _verbose(verbose)
    {
    }

    void begin()
    {
      Serial.printf("SDI12 datapin: %d\n", _SDI12.getDataPin());
      // Serial.println(_verbose);
      _SDI12.begin();
    }
    

    String requestAndReadData(String command, bool trim = false) {

      // delay(300);                   // Wait for response to be ready
      // _SDI12.clearBuffer();
      _SDI12.sendCommand(command); // Send the SDI-12 command
      delay(300);                   // Wait for response to be ready

      String sensorResponse = "";
      //Read the response from the sensor
      while (_SDI12.available()) { // Check if there is data available to read
        char c = _SDI12.read();    // Read a single character
        // if (c != -1 && c != 0x00 && c != 0x7F) {              // Check if the character is valid
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
      // String sensorResponse = _SDI12.readStringUntil('\n');

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
        print_response(command, sensorResponse);
      }

      return sensorResponse;
    }

    String measureConcurrent(String measure_command, uint8_t address)
    {
        measure_command = String(address) + measure_command + "!";
        String measureResponse = requestAndReadData(measure_command, true);  // Command to take a measurement
        // print_response(measure_command, measureResponse);

        // last value is number of measured values
        uint8_t n_values = atoi(measureResponse.end()-1);
        Serial.println(n_values);
        uint8_t total_length = n_values*n_bytes_per_val;
        Serial.println(total_length);

        delay(_meaure_delay);

        // for C commands
        // max message length 75 B
        uint8_t position = 0;
        String data_command = String(address) + "D" + String(position) + "!";
        String sensorResponse = requestAndReadData(data_command, true);

        // print_response(data_command, sensorResponse);

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
        // 30 2B 30 2E 39 39 34 35 2B 31 2E 30 30 34 35 2B 31 2E 30 32 32 39 2B 30 2E 39 38 34 38 2B 30 2E 39 38 35 36 2B 30 2E 39 38 34 38 D A 
        // 30 2B 30 2E 39 39 34 33 2B 31 2E 30 30 35 2B 31 2E 30 32 33 31 2B 30 2E 39 38 34 38 2B 30 2E 39 38 35 33 2B 30 2E 39 38 35 D A
        // 0+0.9945+1.0045+1.0229+0.9848+0.9856+0.9848
        // 0+0.9943+1.005+1.0231+0.9848+0.9853+0.985
        Serial.println(sensorResponse.length());
        Serial.println(sensorResponse.length() - total_length);
        return sensorResponse.substring(sensorResponse.length() - total_length);
    }















    char* requestAndReadData(const char* command, uint8_t* n_bytes) {

      for(int i=0; i<max_msg_length; i++)
        msg_buf[i] = 0;

      _SDI12.sendCommand(command); // Send the SDI-12 command
      delay(300);                   // Wait for response to be ready

      u_int8_t counter = 0;
      //Read the response from the sensor
      while (_SDI12.available()) { // Check if there is data available to read
        char c = _SDI12.read();    // Read a single character
        if (c != -1) {              // Check if the character is valid
          msg_buf[counter] = c;      // Append the character to the response string
          counter++;

          if(_verbose > 1)
            Serial.print(c, HEX); Serial.print(" ");
          if(counter >= max_msg_length)
            break;
        }
        delay(20);  // otherwise it would leave some chars to next message...
      }
      if(_verbose > 1)
        Serial.println("");

      *n_bytes = counter;
      
      if(_verbose > 0)
      {
        // if printf not available (Arduino)
        // char string_buffer[128]; // Buffer to hold the formatted string
        // snprintf(string_buffer, sizeof(string_buffer), "command %s: %s", command.c_str(), sensorResponse.c_str());
        // Serial.println(string_buffer);
        print_response(command, msg_buf);
      }

      return msg_buf;
    }

    char* measureConcurrent(String measure_command, uint8_t address, float* values, uint8_t* n_values)
    {
        uint8_t n_bytes = 0;
        measure_command = String(address) + measure_command + "!";
        requestAndReadData(measure_command.c_str(), &n_bytes);  // Command to take a measurement

        // for(int i=0; i<n_bytes; i++)
        //   Serial.printf("%02X ",msg_buf[i]);
        // Serial.println();

        // last value is the number of measured values
        char n_expected = msg_buf[n_bytes-3]; // nbytes-1, message ends with 0D 0A
        // Serial.println(n_expected);

        delay(_meaure_delay);

        // for C commands
        // max message length 75 B
        uint8_t position = 0;
        String data_command = String(address) + "D" + String(position) + "!";
        requestAndReadData(data_command.c_str(), &n_bytes);  // Command to take a measurement

        char* msg_start = findFirstDigit(msg_buf, n_bytes);
        Serial.printf("cleared msg: %s\n", msg_start);
        
        uint8_t parsed_values = 0; // Number of values successfully parsed
        char* msg_ptr;
        // Extracts the device address and stores a ptr to the rest of the
        // message buffer for use below (to extract values only)
        uint8_t raddress = strtof(msg_start, &msg_ptr);
        // Serial.printf("address: %d\n", raddress);

        char* next_msg_ptr;
        float value;
        // Extract the values from the message buffer and put into user
        // supplied buffer
        for (size_t i = 0; i < n_expected; i++) {
            value = strtof(msg_ptr, &next_msg_ptr);
            if(msg_ptr == next_msg_ptr){
                break;
            }
            // Serial.printf("Value: %f\n", value);
            values[parsed_values++] = value;
            msg_ptr = next_msg_ptr;
        }

        *n_values = parsed_values;
        return msg_start;
    }

  private:
    void print_response(String cmd, String response)
    {
      Serial.printf("command %s: %s\n", cmd.c_str(), response.c_str());
    }
    
    void print_response(String cmd, const char* response)
    {
      Serial.printf("command %s: %s\n", cmd.c_str(), response);
    }

    char* findFirstDigit(char* str) {
      // Loop through each character until we hit the string's null terminator
      while (*str != '\0') {
          // Check if the current character is a digit
          if (*str >= '0' && *str <= '9') {
              return str;  // Return the pointer to the current character
          }
          Serial.printf("skipping %X\n", *str);
          str++;  // Move to the next character
      }
      return nullptr;  // Return nullptr if no digit is found
    }

    char* findFirstDigit(char* str, uint8_t n_bytes) {
      // Loop through each character until we hit the string's null terminator
      for(int i=0; i<n_bytes; i++) {
          // Check if the current character is a digit
          if (*str >= '0' && *str <= '9') {
              return str;  // Return the pointer to the current character
          }
          Serial.printf("skipping %X\n", *str);
          str++;  // Move to the next character
      }
      return nullptr;  // Return nullptr if no digit is found
    }
};
