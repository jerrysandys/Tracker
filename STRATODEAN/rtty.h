#ifndef RTTY_H
#define RTTY_H

#include <Arduino.h>
#include <string.h>
#include <util/crc16.h>

class RTTY {  
public:
  RTTY(uint8_t pin, uint8_t led_pin);
  void send(char *data);
  void send_byte(char c);
  void send_bit(uint8_t bit);
  uint16_t crc16_chksum(char *str);
};

#endif

