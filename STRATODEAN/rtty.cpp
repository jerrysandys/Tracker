#include "rtty.h"

uint8_t _pin = 0;
uint8_t _led_pin = 0;

//Setting our pin assignments
RTTY::RTTY(uint8_t pin, uint8_t led_pin) {
  pinMode(pin, OUTPUT);
  pinMode(_led_pin, OUTPUT);
  _pin = pin;
  _led_pin = led_pin;
}

//Method to send data string
void RTTY::send(char *data) {

  char c;
  char chksum_str[6];
  
  //Call checksum routine
  unsigned int CHECKSUM = crc16_chksum(data);
  sprintf(chksum_str, "*%04X\n", CHECKSUM);
  //Concatinate data with checksum
  strcat(data, chksum_str);
  
  c = *data++;
  //While we haven't reached the end of the data string, keep sending bytes
  while (c != '\0') {
    send_byte(c);
    c = *data++;
  }

}
void RTTY::send_byte(char c) {
  uint8_t i;

  send_bit(0); // Start bit

  // Send bits for for char LSB first	
  for (i=0; i<7; i++) { // Change this here 7 or 8 for ASCII-7 / ASCII-8
    if (c & 1)
      send_bit(1);
    else
      send_bit(0);	
    c = c >> 1;
  }

  send_bit(1); // Stop bit
  send_bit(1); // Stop bit
}

void RTTY::send_bit(uint8_t bit) {
  digitalWrite(_led_pin, HIGH);
  if (bit) {
    // high
    digitalWrite(_pin, HIGH);
  } 
  else {
    // low
    digitalWrite(_pin, LOW);
  }

  //delayMicroseconds(3333); // 300 baud
  //delayMicroseconds(13325); // 75 baud
  delayMicroseconds(10000); // base delay for 45/50
  delayMicroseconds(10000); // 50 baud
  //delayMicroseconds(12222); // 45 baud
  digitalWrite(_led_pin, LOW);
}

//Checksum routine
uint16_t RTTY::crc16_chksum(char *str) 
{
  size_t i;
  uint16_t crc;
  uint8_t c;

  crc = 0xFFFF;

  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(str); i++) 
  {
    c = str[i];
    crc = _crc_xmodem_update (crc, c);
  }
  return crc;
}

