//Include the other classes
#include "SoftwareSerial.h"
#include "gps.h"
#include <stdlib.h>
#include <TinyGPS_UBX.h>
TinyGPS tgps;

//debug flag
#define DEBUG false

//Set up some gps variables
byte gps_hour, gps_minute, gps_second;
long gps_lat, gps_lon;
unsigned long gps_fix_age;
uint8_t led_pin = 0;

//Class instantisation with pin assignments
GPS::GPS(uint8_t rx_pin, uint8_t tx_pin, uint8_t _pin):
SoftwareSerial(rx_pin, tx_pin) {
  pinMode(rx_pin, INPUT);
  pinMode(tx_pin, OUTPUT);
  pinMode(_pin, OUTPUT);
  led_pin = _pin;
  //Start transmission on 9600 baud.
  begin(9600);
}

//Start method
void GPS::start() {
  println(F("GPS Start"));
  // Wait for uBlox to become ready
  delay(2000);

  // Disable all NMEA messages, using $PUBX only
  println(F("$PUBX,40,GLL,0,0,0,0*5C"));
  println(F("$PUBX,40,GGA,0,0,0,0*5A"));
  println(F("$PUBX,40,GSA,0,0,0,0*4E"));
  println(F("$PUBX,40,RMC,0,0,0,0*47"));
  println(F("$PUBX,40,GSV,0,0,0,0*59"));
  println(F("$PUBX,40,VTG,0,0,0,0*5E"));
  println(F("$PUBX,40,ZDA,0,0,0,0*44"));

  delay(2500); // Wait for the GPS to process all commands

  //Set the navigation mode to flight mode
  //This is important - we don't want it dropping out at 18km or similar!
  uint8_t set_nav5[] = { 
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC       };
  //send flight mode command
  send_ubx(set_nav5, sizeof(set_nav5)/sizeof(uint8_t));

  //Get acknowledgment
  while (get_ubx_ack(set_nav5) != true) {
    println(F("false"));
    delay(1000);
  }
  delay(1000);
}

//This is the function that gets called from the loop
char *GPS::get_info() {
  static char info[BUFSIZE] = "";

  //Create variables to hold values that need calculations done before transmitting
  char strTime[20];
  char strLatitude[20];
  char strLongitude[20];
  char strAltitude[20];
  char strFix[3];
  //char strSpeed[20];
  //char strVSpeed[20];
  char strSats[3];
  char strFixQuality[3];
  byte hasfix;

  //Poll GPS and fill TinyGPS with data
  poll();  
  //Call TinyGPS functions for data
  tgps.crack_time(&gps_hour, &gps_minute, &gps_second, &gps_fix_age);
  tgps.get_position(&gps_lat, &gps_lon, &gps_fix_age);

  //Create time string
  sprintf(strTime, "%02d:%02d:%02d", gps_hour, gps_minute, gps_second);

  //Serial.println();
  //Serial.print("time: "); Serial.println(strTime);

  //uses the dtostrf function found in stdlib.h to convert floats to strings
  //Latitude
  dtostrf(gps_lat/100000.0, 0, 5, strLatitude);
  //Serial.print("latitude: "); Serial.println(gps_lat/100000.0, 5);

  //Longitude
  dtostrf(gps_lon/100000.0, 0, 5, strLongitude);
  //Serial.print("longitude: "); Serial.println(gps_lon/100000.0, 5);

  //Altitude
  dtostrf(tgps.altitude()/100, 0, 0, strAltitude);
  //Serial.print("altitude: "); Serial.print(tgps.altitude()/100.0, 0); Serial.println(" m");
  //Serial.print("speed: "); Serial.print(gps.speed()/100.0, 0); Serial.println(" km/h");

  //Vertical Speed
  //sprintf(strVSpeed, "%02d", tgps.vspeed());
  //Serial.print("vert. speed: "); Serial.print(tgps.vspeed(), DEC); Serial.println(" cm/s");

  //Satellites
  sprintf(strSats, "%d", tgps.sats());
  //Serial.print("satellites: "); Serial.println(tgps.sats(), DEC);

  //Whether the GPS has a fix or no
  hasfix = tgps.has_fix();
  sprintf(strFix, "%d", hasfix);
  //light green led if we have a fix
  switch (tgps.has_fix()){
  case 0:
    digitalWrite(led_pin, LOW);
    break;
  case 1:
    digitalWrite(led_pin, HIGH);
    break;
  default:
    digitalWrite(led_pin, LOW);
    break;
  }

  //Serial.print("has fix: "); Serial.println(hasfix);

  //fix quality - can range from 0 - 3. 3 is the best
  sprintf(strFixQuality, "%d", tgps.fix_quality());
  //Serial.print("fix quality: "); Serial.println(strFixQuality);
  //Serial.print("fix age: "); Serial.println(gps_fix_age, DEC);
  //Serial.println("------------");

  //Concatinating it into a string called 'info'
  snprintf(info, 9, "%s", strTime);
  strncat(info, ",", 1);
  strncat(info, strLatitude, 20);  
  strncat(info, ",", 1);
  strncat(info, strLongitude, 20);
  strncat(info, ",", 1);
  strncat(info, strAltitude, 20);    
  strncat(info, ",", 1);
  strncat(info, strFixQuality, 3);
  strncat(info, ",", 1);
  strncat(info, strSats, 3);

  //println(info);

  //Returning the info string back to the loop
  return info;
}

// request uBlox to give fresh data
boolean GPS::poll() {
  //This is the special command that gets the information from the GPS
  //Send this to the GPS and wait for response
  println(F("$PUBX,00*33"));
  //delay(1500);
  unsigned long starttime = millis();

  //Continue to read from gps hardware port
  while (true) {
    if (available()) {
      char c = read();
#if DEBUG
      println(c);
#endif
      if (tgps.encode(c))
        return true;
    }
    // stop waiting
    if (millis() - starttime > 1000) {
#if DEBUG
      println(F("timeout"));
#endif
      break;
    }
  }
  return false;
}

//subroutine to send commands to the GPS
void GPS::send_ubx(uint8_t *msg, uint8_t len) {
  for (uint8_t i = 0; i < len; i++)
    write(msg[i]);
  println();
}

//standard UKHAS function to get acknowledgement from GPS
boolean GPS::get_ubx_ack(uint8_t *msg) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();

  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5; // header
  ackPacket[1] = 0x62; // header
  ackPacket[2] = 0x05; // class
  ackPacket[3] = 0x01; // id
  ackPacket[4] = 0x02; // length
  ackPacket[5] = 0x00;
  ackPacket[6] = msg[2]; // ACK class
  ackPacket[7] = msg[3]; // ACK id
  ackPacket[8] = 0; // CK_A
  ackPacket[9] = 0; // CK_B

  // Calculate the checksums
  for (uint8_t i = 2; i < 8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }

  while (1) {
    if (ackByteID > 9) {
      // All packets in order!
      return true;
    }

    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      return false;
    }

    if (available()) {
      b = read();

      if (b == ackPacket[ackByteID]) { 
        ackByteID++;
      } 
      else { 
        ackByteID = 0; // invalid order
      }
    }
  }
}


