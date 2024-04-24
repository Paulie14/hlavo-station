# Examples

### hlavo-station_rain_counter
Minimalistic example. Reading rain gauge pin and getting ticks.

### hlavo-station_speed_counter
Minimalistic example. Reading anemometer pin and getting ticks.

### hlavo-station_weather
Weather station example.
- reading anemometer (ticks)
- reading rain gauge (ticks)
- reading wind direction
- uses WeatherMeters libs with several fixes (debouncing, ticks , widn direction table)

### hlavo-station_SD
Minimalistic example. Reading and writing to a file on SD card.

### hlavo-station_weather_full
Weather station example.
- adds real time clock (RTC)
- adds light sensor BH1750
- adds battery voltage
- adds MeteoData
- adds SD card writer
- moves some code to lib "Hlavo"

### hlavo-PR2
Minimalistic example. Communication with PR2 using Arduino SDI12 library.
- works fine with Arduino
- can get sensor info
- can send measure request with responce
- does not receive any data after data request

### hlavo-PR2-ESP32
Minimalistic example. Communication with PR2 using ESP32-SDI12 library.
- currently same result as example hlavo-PR2
- can get sensor info
- can send measure request with responce
- does not receive any data (actually requests data in infinite loop)