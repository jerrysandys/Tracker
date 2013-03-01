#include "uno.h"

UNO::UNO(void) {
}

uint8_t UNO::get_bufsize() {
  return BUFSIZE;
}

float UNO::get_voltage() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  //result = 1126400L / result; // Back-calculate AVcc in mV
  
  result = 1100000L / result; // Back-calculate AVcc in mV
  return (float)result;  
}



