#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


namespace hlavo{
  static const size_t max_dirpath_length = 100;
  static const size_t max_filepath_length = 200;
  static const size_t max_csvline_length = 400;

  static void strcat_safe(char* strdest, const char* str)
  {
    // test truncation
    // if (strlen(str) + 1 > sizeof(strdest) - strlen(strdest))
            // Serial.print("onstack would be truncated");

    // keep the ending char '\0' at the end, throw away overflow
    (void)strncat(strdest, str, sizeof(strdest) - strlen(strdest) - 1);
  }
};