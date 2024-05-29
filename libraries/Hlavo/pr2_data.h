#include <RTClib.h> // just for DateTime

#define NUM_VALUES 6

class PR2Data{

  public:
    DateTime datetime;

    // weather station
    float permitivity[NUM_VALUES];
    float soil_moisture[NUM_VALUES];
    float raw_ADC[NUM_VALUES];

    static const char * delimiter;
    static const float NaN;

    void setPermitivity(float* sourceArray, size_t n_values)
    {
      copyArray(permitivity, sourceArray, n_values);
    }

    void setSoilMoisture(float* sourceArray, size_t n_values)
    {
      copyArray(soil_moisture, sourceArray, n_values);
    }

    void setRaw_ADC(float* sourceArray, size_t n_values)
    {
      copyArray(raw_ADC, sourceArray, n_values);
    }

    static char* headerToCsvLine(char* csvLine) {
        // datetime + 3 fields
        const uint8_t n_columns = 1 + NUM_VALUES*3;
        char columnNames[n_columns][20];

        uint8_t j = 0;
        sprintf(columnNames[j++],"DateTime");
        for(uint8_t i=0; i<NUM_VALUES; i++)
          sprintf(columnNames[j++],"Perm_%d", i);
        for(uint8_t i=0; i<NUM_VALUES; i++)
          sprintf(columnNames[j++],"SoilMoistMin_%d", i);
        for(uint8_t i=0; i<NUM_VALUES; i++)
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
    {
      datetime = DateTime(0,0,0, 0,0,0);

      for(uint8_t i=0; i<NUM_VALUES; i++)
      {
        permitivity[i] = 0.0f;
        soil_moisture[i] = 0.0f;
        raw_ADC[i] = 0.0f;
      }
    }

    // Function to convert MeteoData struct to CSV string with a custom delimiter
    char* dataToCsvLine(char* csvLine) {

      const char * dt = datetime.timestamp().c_str();
      sprintf(csvLine, "%s%s", dt, delimiter);
      char number[10];

      for(uint8_t i=0; i<NUM_VALUES; i++){
        sprintf(number,"%.4f%s", permitivity[i], delimiter);
        strcat(csvLine, number);
      }
      for(uint8_t i=0; i<NUM_VALUES; i++){
        sprintf(number,"%.4f%s", soil_moisture[i], delimiter);
        strcat(csvLine, number);
      }
      for(uint8_t i=0; i<NUM_VALUES-1; i++){
        sprintf(number,"%.0f%s", raw_ADC[i], delimiter);
        strcat(csvLine, number);
      }
      // last value without delimiter
      sprintf(number,"%.4f\n", raw_ADC[NUM_VALUES-1]);
      strcat(csvLine, number);
      // strcat(csvLine,"\n");
      
      return csvLine;
    }

  private:
    void copyArray(float* destinationArray, float* sourceArray, size_t n_values)
    {
      memcpy(destinationArray, sourceArray, n_values*sizeof(float));
    }
};

const char * PR2Data::delimiter = ";";
const float PR2Data::NaN = 0.0;
