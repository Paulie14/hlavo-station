# pragma once

#include "pr2_comm.h"
#include "pr2_data.h"

/// @brief Class that keeps track of requesting and receiving PR2 data from a single sensor.
/// The point is not to block the main loop with delays between commands.
/// It uses common timer pr2_delay_timer, which must be global.
class PR2Reader{
  private:
    uint8_t _address;
    PR2Comm _pr2_comm;

    static const uint8_t _n_fields = 3;
    const char* _list_of_commands[_n_fields] = {"C", "C1", "C9"};

    uint8_t icmd = 0;

    float rec_values[10];
    uint8_t rec_n_values = 0;

  public:
    PR2Data data;
    bool finished = false;

    PR2Reader(PR2Comm &pr2_comm, uint8_t address)
    :_address(address), _pr2_comm(pr2_comm)
    {
      Reset();
    }

    void TryRequest()
    {
      if(!pr2_delay_timer.running)
      {
        _pr2_comm.measureRequest(_list_of_commands[icmd], _address);
        pr2_delay_timer.reset();
      }
    }

    void TryRead()
    {
      if(pr2_delay_timer())
      {
        char* res = _pr2_comm.measureRead(_address, rec_values, &rec_n_values);
        // _pr2_comm.print_values("field", rec_values, rec_n_values);
        if(res != nullptr)
        {
          switch(icmd)
          {
            case 0: data.setPermitivity(rec_values, rec_n_values); break;
            case 1: data.setSoilMoisture(rec_values, rec_n_values); break;
            case 2: data.setRaw_ADC(&rec_values[1], rec_n_values-1); break;
          }
        }
        icmd++;

        if(icmd == _n_fields)
        {
          icmd = 0;
          finished = true;
        }
      }
    }

    void Reset()
    {
      icmd = 0;
      finished = false;
      data = PR2Data();
    }

    
};
