//Title:    STRATODEAN Tracker code
//Author:   Mark Ireland
//Website:  http://www.stratodean.co.uk
//Notes:    Thanks goes to all the existing UKHAS enthusiasts who helped with providing example code for others to use.
//          Thanks also to the guys on the UKHAS #highaltitude irc channel for their invaluable help.

//We need SoftwareSerial port as we are using the hardware port for GPS
#include <SoftwareSerial.h>
//This is a modifed version of the TinyGPS libaray for UBlox GPS chips
#include <TinyGPS_UBX.h>
//Libaray for reading and writing to SD card
#include <SdFat.h>
//Include our separate code for ease of reading
#include "rtty.h"
#include "gps.h"
//Include temperature
#include <OneWire.h>
#include <DallasTemperature.h>

//Setup pins
#define GPSRX 2
#define GPSTX 6
#define NTX2 3
#define REDLED 4
#define GREENLED 5
#define TEMP 7
//Character buffer for transmission
#define DATASIZE 128
char data[DATASIZE];
//Character buffer for battery voltage
#define BUFSIZE 16
char battery[BUFSIZE];
char internalT[BUFSIZE];
char externalT[BUFSIZE];
//s_id as sentence id to have a unique number for each transmission
uint16_t s_id = 0;
//Select pin for SD card write
const uint8_t chipSelect = SS;
//boolean to detect whether we should write to the SD card or not - don't want to keep trying if we
//already know that a card is not present.
boolean sdWrite = true;

//initialise our classes
RTTY rtty(NTX2, REDLED);
GPS gps(GPSRX, GPSTX, GREENLED);
SdFat sd;
OneWire oneWire(TEMP);
DallasTemperature sensors(&oneWire);

DeviceAddress internalTemp = {0x28, 0xCC, 0x5C, 0x9D, 0x04, 0x00, 0x00, 0xAA};
DeviceAddress externalTemp = {0x28, 0xAE, 0x2F, 0xAA, 0x04, 0x00, 0x00, 0xEA};
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
  
  //Initialise Temperature Sensors
  sensors.begin();
  sensors.setResolution(internalTemp, 10);
  sensors.setResolution(externalTemp, 10);
  
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
  }
  else
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

  //Get battery voltage
  dtostrf(get_voltage()/1000,0,1, battery);
  //Get temperature
  sensors.requestTemperatures();
  dtostrf(sensors.getTempC(internalTemp),0,2,internalT);
  dtostrf(sensors.getTempC(externalTemp),0,2,externalT);

  //How much ram do we have
  Serial.println(freeRam());  

  //$$callsign,sentence_id,time,latitude,longitude,altitude,fix,ascentrate,satellites,batteryvoltage*CHECKSUM\n
  
  //Call gps.get_info and, along with the s_id and battery, put it altogether into the string called 'data'
  snprintf(data, DATASIZE, "$$SDEAN,%d,%s,%s,%s,%s", s_id, gps.get_info(), internalT, externalT, battery);
  //print this to the screen and the ram
  Serial.println(data);
  Serial.println(freeRam());
  //write this data string to the SD card if we are using it
  if (sdWrite) logEvent(data);
  //send the data over the radio!
  rtty.send(data);
  Serial.println(freeRam());
  //increment the id next time.
  s_id++;
}

//*************************************************************************************
//
//helper routines()
//
//*************************************************************************************

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

//subroutine to give the amount of ram available on the board
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

//Subroutine to give current battery voltage
//Found this from "http://code.google.com/p/tinkerit/wiki/SecretVoltmeter"
float get_voltage() {
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

//Subroutine to flash the LEDs
//This helps us know if the SD card is working when we turn the tracker on.
void flashLEDs() {
  //Use this to flash LEDs when sd card is available and writeable
  digitalWrite(GREENLED, LOW);
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
