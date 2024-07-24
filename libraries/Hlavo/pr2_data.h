# pragma once

#include <RTClib.h> // just for DateTime
#include "data_base.h"
#include "common.h"
using namespace hlavo;

/// @brief Data class for handling PR2 data from a single sensor.
class PR2Data : public DataBase{
  public:
    static const uint8_t size = 6;

    float permitivity[size];
    float soil_moisture[size];
    float raw_ADC[size];

    static char* headerToCsvLine(char* csvLine);

    PR2Data();
    char* dataToCsvLine(char* csvLine) const override;
    char* print(char* msg_buf) const ;

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

  private:
    void copyArray(float* destinationArray, float* sourceArray, uint8_t n_values)
    {
      memcpy(destinationArray, sourceArray, n_values*sizeof(float));
    }
};


char* PR2Data::headerToCsvLine(char* csvLine) {
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
      strcat_safe(csvLine, columnNames[i]);

      // If it's not the last string, add the delimiter
      if (i < n_columns - 1)
        strcat_safe(csvLine, delimiter);
      else
        strcat_safe(csvLine, "\n");
  }

  return csvLine;
}

PR2Data::PR2Data()
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
char* PR2Data::dataToCsvLine(char* csvLine) const {

  const char * dt = datetime.timestamp().c_str();
  snprintf(csvLine, sizeof(csvLine), "%s%s", dt, delimiter);
  char number[10];

  for(uint8_t i=0; i<size; i++){
    snprintf(number, sizeof(number), "%.4f%s", permitivity[i], delimiter);
    strcat_safe(csvLine, number);
  }
  for(uint8_t i=0; i<size; i++){
    snprintf(number, sizeof(number), "%.4f%s", soil_moisture[i], delimiter);
    strcat_safe(csvLine, number);
  }
  for(uint8_t i=0; i<size-1; i++){
    snprintf(number, sizeof(number), "%.0f%s", raw_ADC[i], delimiter);
    strcat_safe(csvLine, number);
  }
  // last value without delimiter
  snprintf(number, sizeof(number), "%.0f\n", raw_ADC[size-1]);
  strcat_safe(csvLine, number);
  // strcat(csvLine,"\n");

  return csvLine;
}

// Print MeteoData
char* PR2Data::print(char* msg_buf) const {

  const char * dt = datetime.timestamp().c_str();
  snprintf(msg_buf, sizeof(msg_buf), "%s\n", dt);
  char number[10];

  strcat_safe(msg_buf, "    Perm. ");
  for(uint8_t i=0; i<size; i++){
    snprintf(number, sizeof(number), "%.4f, ", permitivity[i]);
    strcat_safe(msg_buf, number);
  }
  strcat_safe(msg_buf, "\n    SoilM. ");
  for(uint8_t i=0; i<size; i++){
    snprintf(number, sizeof(number), "%.4f, ", soil_moisture[i]);
    strcat_safe(msg_buf, number);
  }
  strcat_safe(msg_buf, "\n    RawADC. ");
  for(uint8_t i=0; i<size-1; i++){
    sprintf(number,"%.0f, ", raw_ADC[i]);
    strcat_safe(msg_buf, number);
  }
  // last value without delimiter
  snprintf(number, sizeof(number), "%.0f", raw_ADC[size-1]);
  strcat_safe(msg_buf, number);
  return msg_buf;
}
