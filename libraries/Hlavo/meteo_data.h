#include <RTClib.h>

struct MeteoData{

  DateTime datetime;

  // weather station
  float wind_direction;
  unsigned int wind_speed_ticks;
  unsigned int raingauge_ticks;

  // RTC
  float humidity;
  float temperature;

  // battery - ESP32 analog read
  float battery_voltage;
};


// Function to convert MeteoData struct to CSV string with a custom delimiter
char* meteoDataToCSV(struct MeteoData data, char* csvLine) {

  char delimiter = ';';

  const char * datetime = data.datetime.timestamp().c_str();
  // Format datetime in "YYYY-MM-DD HH:MM:SS" format
  // sprintf(datetime, "%04d-%02d-%02d %02d:%02d:%02d%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
  //         data.datetime.year(), data.datetime.month(), data.datetime.day(),
  //         data.datetime.hour(), data.datetime.minute(), data.datetime.second());
  sprintf(csvLine, "%s%c%.2f%c%u%c%u%c%.2f%c%.2f%c%.2f\n",
          datetime, delimiter,
          data.wind_direction, delimiter,
          data.wind_speed_ticks, delimiter,
          data.raingauge_ticks, delimiter,
          data.humidity, delimiter,
          data.temperature, delimiter,
          data.battery_voltage);
  return csvLine;
}
