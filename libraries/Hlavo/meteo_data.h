#include <RTClib.h> // just for DateTime

class MeteoData{

  public:
    DateTime datetime;

    // weather station
    float wind_direction;
    unsigned int wind_speed_ticks;
    unsigned int raingauge_ticks;

    // RTC
    float humidity_mean, humidity_var;
    float temperature_mean, temperature_var;

    float light_mean, light_var;

    // battery - ESP32 analog read
    float battery_voltage_mean, battery_voltage_var;

    static const char * delimiter;

    static char* headerToCsvLine(char* csvLine) {
        const int n_columns = 8;
        const char* columnNames[] = {
          "DateTime",
          "WindDirection",
          "WindSpeedTicks",
          "RainGaugeTicks",
          "Humidity_Mean",
          "Humidity_Var",
          "Temperature_Mean",
          "Temperature_Var",
          "Light_Mean",
          "Light_Var",
          "BatteryVoltage_Mean",
          "BatteryVoltage_Var"
        };
        csvLine[0] = '\0'; // Initialize the CSV line as an empty string

        // Iterate through the array of strings
        for (int i = 0; i < n_columns; ++i) {
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

    MeteoData()
    {
      datetime = DateTime(0,0,0, 0,0,0);

      wind_direction = 0.0f;
      wind_speed_ticks = 0;
      raingauge_ticks = 0;

      humidity_mean = humidity_var = 0.0f;
      temperature_mean = temperature_var = 0.0f;
      light_mean = light_var = 0.0f;

      battery_voltage_mean = battery_voltage_var = 0.0f;
    }

    // Function to convert MeteoData struct to CSV string with a custom delimiter
    char* dataToCsvLine(char* csvLine) {

      const char * dt = datetime.timestamp().c_str();
      // Format datetime in "YYYY-MM-DD HH:MM:SS" format
      // sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
      //         data.datetime.year(), data.datetime.month(), data.datetime.day(),
      //         data.datetime.hour(), data.datetime.minute(), data.datetime.second());
      sprintf(csvLine, "%s%s%.1f%s%u%s%u%s"
              "%.2f%s%.2f%s" // humidity
              "%.2f%s%.2f%s" // temperature
              "%.0f%s%.0f%s" // light
              "%.3f%s%.3f\n",// battery
              dt, delimiter,
              wind_direction, delimiter,
              wind_speed_ticks, delimiter,
              raingauge_ticks, delimiter,
              humidity_mean, delimiter,
              humidity_var, delimiter,
              temperature_mean, delimiter,
              temperature_var, delimiter,
              light_mean, delimiter,
              light_var, delimiter,
              battery_voltage_mean, delimiter,
              battery_voltage_var);
      return csvLine;
    }
};

const char * MeteoData::delimiter = ";";

// // Function to convert MeteoData struct to CSV string with a custom delimiter
// char* meteoDataToCSV(struct MeteoData data, char* csvLine) {

//   char delimiter = ';';

//   const char * datetime = data.datetime.timestamp().c_str();
//   // Format datetime in "YYYY-MM-DD HH:MM:SS" format
//   // sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
//   //         data.datetime.year(), data.datetime.month(), data.datetime.day(),
//   //         data.datetime.hour(), data.datetime.minute(), data.datetime.second());
//   sprintf(csvLine, "%s%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
//           datetime, delimiter,
//           data.wind_direction, delimiter,
//           data.wind_speed_ticks, delimiter,
//           data.raingauge_ticks, delimiter,
//           data.humidity, delimiter,
//           data.temperature, delimiter,
//           data.battery_voltage);
//   return csvLine;
// }
