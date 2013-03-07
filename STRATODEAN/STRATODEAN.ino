/*
STRATODEAN Tracker code
Mark Ireland
*/

//Setup pins
#define GPSRX 2
#define GPSTX 6
#define NTX2 3
#define REDLED 4
#define GREENLED 5

//We need SoftwareSerial port as we are using the hardware port for GPS
#include <SoftwareSerial.h>
//This is a modifed version of the TinyGPS libaray for UBlox GPS chips
#include <TinyGPS_UBX.h>
//Libaray for reading and writing to SD card
#include <SdFat.h>

//Include our separate code for ease of reading
#include "rtty.h"
#include "gps.h"
#include "uno.h"

//Size of our transmission
#define DATASIZE 128
char data[DATASIZE];
//s_id as sentence id to have a unique number for each transmission
uint16_t s_id = 0;

//initialise our our code
RTTY rtty(NTX2, REDLED);
GPS gps(GPSRX, GPSTX, GREENLED);
UNO uno;
SdFat sd;

//Select pin for SD card write
const uint8_t chipSelect = SS;
//boolean to detect whether we should write to the SD card or not - don't want to keep trying if we
//already know that a card is not present.
boolean sdWrite = true;

//*************************************************************************************
//
//setup()
//
//*************************************************************************************

void setup() {
  //Set the pin modes for the LEDs and SD card
  pinMode(REDLED, OUTPUT);  
  pinMode(GREENLED, OUTPUT);
  pinMode(10, OUTPUT);
  
  //Intialise our hardware serial port to talk to the GPS at 9600 baud. 
  Serial.begin(9600);
  Serial.println(F("STRATODEAN Payload Tracker, initialising......"));
  //Check how much RAM we have
  Serial.println(freeRam());
  
  //Light up both LEDs to indicate that we have a go situation!
  digitalWrite(GREENLED, HIGH);
  digitalWrite(REDLED, HIGH);  
  //Initialise GPS
  gps.start();
  //Initialise SD card
  //Try and write to the SD card - if we can then fine, if we can't then set sdWrite to false
  //so we don't try to write to it later
  if (!sd.begin(chipSelect, SPI_HALF_SPEED)){
    sdWrite = false;
     Serial.println(F("no write"));
  }else
  {
     Serial.println(F("write"));
     //use logEvent subroutine to indicate new transmission with *
     logEvent("*");
     //call flashLEDs to flash the LEDs if we can write to the SD card
     flashLEDs();
  }
  digitalWrite(GREENLED, LOW);
  digitalWrite(REDLED, LOW);  
  Serial.println(F("GPS and SD initialised"));  
}

//*************************************************************************************
//
//loop()
//
//*************************************************************************************

void loop() {
  
  //Get battery voltage from the UNO class
  char battery[uno.get_bufsize()];
  dtostrf(uno.get_voltage()/1000,0,1, battery);
  
  //How much ram do we have
  Serial.println(freeRam());  
  /* $$callsign,sentence_id,time,latitude,longitude,altitude,fix,ascentrate,satellites,
        batteryvoltage*CHECKSUM\n  */
  //Call gps.get_info and, along with the s_id and battery, put it altogether into the string called 'data'
  snprintf(data, DATASIZE, "$$SDEAN1,%d,%s,%s", s_id, gps.get_info(), battery);
  //print this to the and the ram
  Serial.println(data);
  Serial.println(freeRam());
  //write this data string to the SD card if we are using it
  if (sdWrite) logEvent(data);
  //send the data over the radio!
  rtty.send(data);
  Serial.println(freeRam());
  //increment the id for next time.
  s_id++;
  delay(500);
}

//subroutine to give the amount of ram available on the board
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

//subroutine to log the sentences to the SD card
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


