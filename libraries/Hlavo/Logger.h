#pragma once

#include <RTClib.h>
#include "SD.h"
#include "file_info.h"
#include "clock.h"
#include <stdarg.h>
#include "common.h"

/// @brief Wrapper class for SD card and CSV file handling.
class Logger
{
  public:
    enum MessageType { INFO, WARN, ERROR };
    static void setup_log(Clock &clock, const char* dir_name);

    static void print(const char* msg, MessageType type = INFO);
    static void print(const String& msg, MessageType type = INFO);
    static void printHex(const char* data, size_t length, MessageType type = INFO);
    static void printf(MessageType type, const char* format, ...);

    static void cleanup_old_logs(int retentionDays = 7);

  private:
    // static const int _retentionDays = 7;
    static Clock* _rtc_clock;
    static char _logDirectory[hlavo::max_dirpath_length];
    static FileInfo _logfile;
    static const int log_msg_maxsize = 550;
    static char _log_buf[log_msg_maxsize];

    static void clean_buf();
    static void createLogFileName();
    static String messageTypeToString(MessageType type);
};

// Static member initialization
Clock* Logger::_rtc_clock = nullptr;
char Logger::_logDirectory[hlavo::max_dirpath_length] = "/logs";
FileInfo Logger::_logfile = FileInfo(SD, "/logs/hlavo_station.log");
char Logger::_log_buf[log_msg_maxsize] = "";




void Logger::setup_log(Clock &clock, const char* dir_name)
{
  _rtc_clock = &clock;

  snprintf(_logDirectory, hlavo::max_dirpath_length, "/%s", dir_name);

  if (!SD.exists(_logDirectory)) {
      SD.mkdir(_logDirectory);
  }

  createLogFileName();
}

void Logger::clean_buf()
{
  for(int i=0; i<log_msg_maxsize; i++)
    _log_buf[i] = '\0';
}

void Logger::print(const char* msg, MessageType type) {
    clean_buf();
    DateTime now = _rtc_clock->now();

    snprintf(_log_buf, sizeof(_log_buf), "%s: [%s] %s\n", now.timestamp().c_str(), messageTypeToString(type), msg);
    Serial.print(_log_buf);
    _logfile.append(_log_buf);
}

void Logger::print(const String& msg, MessageType type) {
    print(msg.c_str());
}

void Logger::printHex(const char* data, size_t length, MessageType type) {
    String hexString = "HEX: ";
    char buf[4]; // Enough to store "FF "
    for (size_t i = 0; i < length; i++) {
        snprintf(buf, sizeof(buf), "%02X ", static_cast<unsigned char>(data[i]));
        hexString += buf;
    }
    print(hexString, type);
}

void Logger::printf(MessageType type, const char* format, ...) {
    clean_buf();
    va_list args;
    va_start(args, format);

    // print start
    DateTime now = _rtc_clock->now();
    char head[50];
    snprintf(head, sizeof(head), "%s: [%s] ", now.timestamp().c_str(), messageTypeToString(type));
    snprintf(_log_buf, sizeof(_log_buf), "%s\n", head);
    size_t offset = strlen(_log_buf);

    vsnprintf(_log_buf + offset, sizeof(_log_buf)-offset, format, args);
    va_end(args);

    Serial.println(_log_buf);
    _logfile.append(_log_buf);
}

void Logger::cleanup_old_logs(int retentionDays) {
    File root = SD.open(_logDirectory);
    if (!root) {
        Serial.println("Failed to open log directory.");
        return;
    }

    root.rewindDirectory();
    File file = root.openNextFile();
    while (file) {
        String fileName = String(file.name());
        if (fileName.endsWith("_hlavo_station.log")) {
            // Extract the timestamp from the file name
            String timestamp = fileName.substring(fileName.lastIndexOf('/') + 1, fileName.lastIndexOf('_'));
            // Parse the date (format: YYYYMMDD)
            int year = timestamp.substring(0, 4).toInt();
            int month = timestamp.substring(4, 6).toInt();
            int day = timestamp.substring(6, 8).toInt();

            DateTime fileDate(year, month, day);
            DateTime currentDate = _rtc_clock->now();
            TimeSpan fileAge = currentDate - fileDate;

            if (fileAge.days() > retentionDays) {
                SD.remove(String(_logDirectory) + "/" + fileName);
                Serial.println("Deleted old log file: " + fileName);
            }
        }
        file = root.openNextFile();
    }
    root.close();
}



void Logger::createLogFileName() {
    DateTime now = _rtc_clock->now();
    char buf[hlavo::max_filepath_length];
    snprintf(buf, sizeof(buf), "%s/%04d%02d%02d_hlavo_station.log", _logDirectory, now.year(), now.month(), now.day());
    Serial.println(buf);
    _logfile = FileInfo(SD, buf);
}

String Logger::messageTypeToString(MessageType type) {
    switch (type) {
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}
