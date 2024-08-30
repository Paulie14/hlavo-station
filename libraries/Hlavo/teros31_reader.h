# pragma once

#include "sdi12_comm.h"

#include "common.h"
using namespace hlavo;
#include "data_base.h"

/// @brief Data class for handling PR2 data from a single sensor.
class Teros31Data : public DataBase{
  public:
    float temperature;
    float pressure;  

    static char* headerToCsvLine(char* csvLine, size_t size);

    Teros31Data();
    char* dataToCsvLine(char* csvLine, size_t size) const override;
    char* print(char* msg_buf, size_t size) const;
};


char* Teros31Data::headerToCsvLine(char* csvLine, size_t size) {
  strcat_safe(csvLine, size, "DateTime");
  strcat_safe(csvLine, size, delimiter);
  strcat_safe(csvLine, size, "Temperature");
  strcat_safe(csvLine, size, delimiter);
  strcat_safe(csvLine, size, "Pressure");
  strcat_safe(csvLine, size, "\n");

  return csvLine;
}

Teros31Data::Teros31Data()
  : DataBase()
{
  temperature = 0;
  pressure = 0;
}

// Function to convert PR2Data struct to CSV string with a custom delimiter
char* Teros31Data::dataToCsvLine(char* csvLine, size_t size) const {

  const char * dt = datetime.timestamp().c_str();
  snprintf(csvLine, size, "%s%s", dt, delimiter);
  char number[10];

  snprintf(number, sizeof(number), "%.4f%s", temperature, delimiter);
  strcat_safe(csvLine, size, number);
  snprintf(number, sizeof(number), "%.4f", pressure);
  strcat_safe(csvLine, size, number);

  return csvLine;
}

// Print PR2Data
char* Teros31Data::print(char* msg_buf, size_t size) const {

  const char * dt = datetime.timestamp().c_str();
  snprintf(msg_buf, size, "%s\n", dt);
  char number[10];

  strcat_safe(msg_buf, size, "Press.: ");
  snprintf(number, sizeof(number), "%.4f", pressure);
  strcat_safe(msg_buf, size, number);
  
  strcat_safe(msg_buf, size, " Temp.: ");
  snprintf(number, sizeof(number), "%.4f", temperature);
  strcat_safe(msg_buf, size, number);

  return msg_buf;
}


/// @brief Class that keeps track of requesting and receiving PR2 data from a single sensor.
/// The point is not to block the main loop with delays between commands.
/// It uses common timer pr2_delay_timer, which must be global.
class Teros31Reader{
  private:
    uint8_t _address;
    SDI12Comm * _sdi12_comm;

    float rec_values[3];
    uint8_t rec_n_values = 0;

  public:
    Teros31Data data;
    bool finished = false;

    Teros31Reader(SDI12Comm* sdi12_comm, uint8_t address);
    bool TryRequest();
    void TryRead();
    void Reset();
};


Teros31Reader::Teros31Reader(SDI12Comm* sdi12_comm, uint8_t address)
  :_address(address), _sdi12_comm(sdi12_comm)
{
  Reset();
}

bool Teros31Reader::TryRequest()
{
  if(!sdi12_delay_timer.running)
  {
    bool res = false;
    _sdi12_comm->measureRequest("C", _address, &res);
    if(res)
      sdi12_delay_timer.reset();
    return res;
  }
  return true;
}

void Teros31Reader::TryRead()
{
  if(sdi12_delay_timer())
  {
    char* res = _sdi12_comm->measureRead(_address, rec_values, &rec_n_values);
    // _sdi12_comm.print_values("field", rec_values, rec_n_values);
    if(res != nullptr)
    {
      if(rec_n_values>0)
        data.pressure = rec_values[0];
      if(rec_n_values>1)
        data.temperature = rec_values[1];
      finished = true;
    }
  }
}

void Teros31Reader::Reset()
{
  finished = false;
  data = Teros31Data();
}
