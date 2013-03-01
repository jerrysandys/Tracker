#ifndef UNO_H
#define UNO_H

#include <Arduino.h>

#define BUFSIZE 16

class UNO {
  public:
    UNO(void);
        uint8_t get_bufsize();
    float get_voltage();

};

#endif
