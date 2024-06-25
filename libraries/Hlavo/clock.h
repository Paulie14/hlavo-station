#include <Wire.h>
#include <RTClib.h>
#include <ESP32Time.h>


class Clock {
  private:
    RTC_DS3231 rtc;
    const uint8_t _data_pin;  // I2C data pin
    const uint8_t _clock_pin; // I2C clock pin
    const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    ESP32Time internal_rtc;

  public:
    // Constructor
    Clock(uint8_t data_pin, uint8_t clock_pin) 
    : _data_pin(data_pin), _clock_pin(clock_pin)
    {}

    // Initialize the clock
    void begin() {
      Serial.println("Initializing RTC...");

      Wire.begin(_data_pin, _clock_pin);

      if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        // Serial.flush();
        // while (1) delay(10);
      }
      if (rtc.lostPower())
      {
        DateTime dt = DateTime(F(__DATE__), F(__TIME__));
        Serial.printf("RTC lost power. Setting time: %s\n", dt.timestamp().c_str());
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(dt);//rtc.adjust(DateTime(2024, 3, 01, 10, 15, 0));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      }
      // rtc.adjust(DateTime(2024, 6, 17, 9, 53, 0));

      //Update internal RTC
      internal_rtc.setTime(rtc.now().unixtime());
    }

    RTC_DS3231& get_rtc()
    {
      return rtc;
    }

    // Get the current date and time from the clock
    DateTime now() {
      return rtc.now();
    }
};