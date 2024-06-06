#pragma once

#include <RTClib.h>

class DataBase
{
  public:
    DateTime datetime;
    static const char * delimiter;
    static const float NaN;
    virtual char* dataToCsvLine(char* csvLine) const = 0;

    DataBase()
    {
      datetime = DateTime(0,0,0, 0,0,0);
    }
};

const char * DataBase::delimiter = ";";
const float DataBase::NaN = 0.0;
