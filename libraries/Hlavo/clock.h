#pragma once

#include <RTClib.h>
#include <ESP32Time.h>


class Clock {
  private:
    RTC_DS3231 rtc;
    const char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    ESP32Time internal_rtc;

  public:
    // Constructor
    Clock()
    {}

    // Initialize the clock
    bool begin() {
      Serial.println("Initializing RTC...");

      if (!rtc.begin()) {
        return false;
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
      return true;
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