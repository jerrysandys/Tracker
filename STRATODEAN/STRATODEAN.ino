/*
STRATODEAN Tracker code
Mark Ireland
*/

#define GPSRX 2
#define GPSTX 6
#define NTX2 3
#define REDLED 4
#define GREENLED 5

#include <SoftwareSerial.h>
#include "rtty.h"
#include "gps.h"
#include "uno.h"
#include <TinyGPS_UBX.h>
#include <SdFat.h>

#define DATASIZE 256

char data[DATASIZE];
uint16_t s_id = 0;

RTTY rtty(NTX2, REDLED);
GPS gps(GPSRX, GPSTX, GREENLED);
UNO uno;
SdFat sd;

const uint8_t chipSelect = SS;
boolean sdWrite = true;


void setup() {
  
  pinMode(REDLED, OUTPUT);  
  pinMode(GREENLED, OUTPUT);
  pinMode(10, OUTPUT);
  Serial.begin(9600);
  //Serial.println(F("STRATODEAN Payload Tracker, initialising......"));
  Serial.println(freeRam());

  digitalWrite(GREENLED, HIGH);
  digitalWrite(REDLED, HIGH);  
  //Initialise GPS
  gps.start();
  //Initialise SD card
  //if (!sd.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sdWrite = false;
     Serial.println(F("no write"));
  }else
  {
     Serial.println(F("write"));
    logEvent("*");
    flashLEDs();
  }
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);  
  //Serial.println(F("GPS initialised"));  
}

void loop() {
  
  char battery[uno.get_bufsize()];
  dtostrf(uno.get_voltage()/1000,0,1, battery);
  
  Serial.println(freeRam());  
  /* $$callsign,sentence_id,time,latitude,longitude,altitude,fix,speed,ascentrate,satellites,
        batteryvoltage*CHECKSUM\n  */
  snprintf(data, DATASIZE, "$$SDEAN1,%d,%s,%s", s_id, gps.get_info(), battery);
  Serial.println(data);
  Serial.println(freeRam());
  if (sdWrite) logEvent(data);
  rtty.send(data);
  Serial.println(freeRam());
  s_id++;
  delay(500);
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void flashLEDs() {
  //Use this to flash LEDs when sd card is available and writeable
  digitalWrite(GREENLED, LOW);\
  digitalWrite(REDLED, LOW);  
  delay(50);
  digitalWrite(GREENLED, HIGH);
  digitalWrite(REDLED, HIGH);  
  delay(150);
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);  
  delay(50);
  digitalWrite(GREENLED, HIGH);
  digitalWrite(REDLED, HIGH);  
  delay(150);
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);  
}

void logEvent(const char *msg) {
  // create dir if needed
  //sd.mkdir("");

  // create or open a file for append
  ofstream sdlog("SDLOG.TXT", ios::out | ios::app);

  // append a line to the file
  sdlog << msg << endl;

  // check for errors
  //if (!sdlog) sd.errorHalt("append failed");

  sdlog.close();
}


