#pragma once

#include <RTClib.h>
#include "SD.h"
#include "file_info.h"
#include "data_base.h"

/// @brief Wrapper class for SD card and CSV file handling.
class CSVHandler
{
  public:
    static void createFile(char* user_filename, char* header, const DateTime& dt)
    {
      char dt_buf[20];
      sprintf(dt_buf, "YY-MM-DD_hh-mm-ss");
      dt.toString(dt_buf);

      char temp_filename[100];
      sprintf(temp_filename, "/%s_%s", dt_buf, user_filename);
      strcpy(user_filename, temp_filename);

      Serial.printf("Creating file: %s\n", user_filename);
      FileInfo datafile(SD, user_filename);
      datafile.write(header);
    }

    static void appendData(char* filename, DataBase* data)
    {
      char csvLine[400];
      File file = SD.open(filename, FILE_APPEND);
      if(!file){
          Serial.println("Failed to open file for appending");
      }
      else
      {
        data->dataToCsvLine(csvLine);
        // Serial.println(csvLine);

        bool res = file.print(csvLine);
        if(!res){
            Serial.println("Append failed");
        }
      }
      file.close();
    }

    static void appendData(char* filename, DataBase* data[], uint8_t n_data)
    {
      //Serial.printf("appendData: N %d\n", n_data);
      char csvLine[400];
      File file = SD.open(filename, FILE_APPEND);
      if(!file){
          Serial.println("Failed to open file for appending");
      }
      else
      {
        for(int i=0; i<n_data; i++)
        {
          // Serial.printf("appendData: %d\n", i);
          // Serial.printf("data: %s\n", data[i].datetime.timestamp().c_str());

          data[i]->dataToCsvLine(csvLine);
          // Serial.println(csvLine);

          bool res = file.print(csvLine);
          if(!res){
              Serial.println("Append failed");
          }
        }
      }
      file.close();
    }

    static void printFile(char* filename)
    {
      FileInfo datafile(SD, filename);
      datafile.read();
    }
};