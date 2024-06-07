# pragma once

#include <RTClib.h> // just for DateTime
#include "data_base.h"

/// @brief Data class for handling PR2 data from a single sensor.
class PR2Data : public DataBase{
  public:
    static const uint8_t size = 6;

    float permitivity[size];
    float soil_moisture[size];
    float raw_ADC[size];

    void setPermitivity(float* sourceArray, uint8_t n_values)
    {
      copyArray(permitivity, sourceArray, n_values);
    }

    void setSoilMoisture(float* sourceArray, uint8_t n_values)
    {
      copyArray(soil_moisture, sourceArray, n_values);
    }

    void setRaw_ADC(float* sourceArray, uint8_t n_values)
    {
      copyArray(raw_ADC, sourceArray, n_values);
    }

    static char* headerToCsvLine(char* csvLine) {
        // datetime + 3 fields
        const uint8_t n_columns = 1 + size*3;
        char columnNames[n_columns][20];

        uint8_t j = 0;
        sprintf(columnNames[j++],"DateTime");
        for(uint8_t i=0; i<size; i++)
          sprintf(columnNames[j++],"Perm_%d", i);
        for(uint8_t i=0; i<size; i++)
          sprintf(columnNames[j++],"SoilMoistMin_%d", i);
        for(uint8_t i=0; i<size; i++)
          sprintf(columnNames[j++],"rawADC_%d", i);
        
        csvLine[0] = '\0'; // Initialize the CSV line as an empty string

        // Iterate through the array of strings
        for (uint8_t i = 0; i < n_columns; ++i) {
            // Concatenate the current string to the CSV line
            strcat(csvLine, columnNames[i]);

            // If it's not the last string, add the delimiter
            if (i < n_columns - 1)
              strcat(csvLine, delimiter);
            else
              strcat(csvLine, "\n");
        }

        return csvLine;
    }

    PR2Data()
    : DataBase()
    {
      for(uint8_t i=0; i<size; i++)
      {
        permitivity[i] = 0.0f;
        soil_moisture[i] = 0.0f;
        raw_ADC[i] = 0.0f;
      }
    }

    // Function to convert MeteoData struct to CSV string with a custom delimiter
    char* dataToCsvLine(char* csvLine) const override {

      const char * dt = datetime.timestamp().c_str();
      sprintf(csvLine, "%s%s", dt, delimiter);
      char number[10];

      for(uint8_t i=0; i<size; i++){
        sprintf(number,"%.4f%s", permitivity[i], delimiter);
        strcat(csvLine, number);
      }
      for(uint8_t i=0; i<size; i++){
        sprintf(number,"%.4f%s", soil_moisture[i], delimiter);
        strcat(csvLine, number);
      }
      for(uint8_t i=0; i<size-1; i++){
        sprintf(number,"%.0f%s", raw_ADC[i], delimiter);
        strcat(csvLine, number);
      }
      // last value without delimiter
      sprintf(number,"%.0f\n", raw_ADC[size-1]);
      strcat(csvLine, number);
      // strcat(csvLine,"\n");
      
      return csvLine;
    }

    // Print MeteoData
    char* print(char* msg_buf) const {

      const char * dt = datetime.timestamp().c_str();
      sprintf(msg_buf, "%s\n", dt);
      char number[10];

      strcat(msg_buf, "    Perm. ");
      for(uint8_t i=0; i<size; i++){
        sprintf(number,"%.4f, ", permitivity[i]);
        strcat(msg_buf, number);
      }
      strcat(msg_buf, "\n    SoilM. ");
      for(uint8_t i=0; i<size; i++){
        sprintf(number,"%.4f, ", soil_moisture[i]);
        strcat(msg_buf, number);
      }
      strcat(msg_buf, "\n    RawADC. ");
      for(uint8_t i=0; i<size-1; i++){
        sprintf(number,"%.0f, ", raw_ADC[i]);
        strcat(msg_buf, number);
      }
      // last value without delimiter
      sprintf(number,"%.0f", raw_ADC[size-1]);
      strcat(msg_buf, number);
      return msg_buf;
    }

  private:
    void copyArray(float* destinationArray, float* sourceArray, uint8_t n_values)
    {
      memcpy(destinationArray, sourceArray, n_values*sizeof(float));
    }
};
