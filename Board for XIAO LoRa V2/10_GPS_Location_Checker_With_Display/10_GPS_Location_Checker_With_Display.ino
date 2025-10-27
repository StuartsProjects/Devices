/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/02/23

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/


/*******************************************************************************************************
  Program Operation -  This program is a portable GPS checker for the Seeduino XIAO with a display option.
  The display is an SSD1306 or SH1106 128x64 I2C OLED. The program reads characters from the GPS for 5
  seconds checking for a location fix and copies the characters from the GPS to the serial monitor so you
  can see if the GPS is working. If there is no GPS location fix seen in the 5 seconds that is reported
  onto the OLED display and IDE serial monitor. This is an example printout in the serial monitor from a
  working GPS with the XIAO having just been powered on;

  9_GPS_Location_Checker_With_Display

  Wait GPS Fix 5 seconds
  $GPGGA,235945.020,,,,,0,0,,,M,,M,,*46
  $GPGLL,,,,,235945.020,V,N*74
  $GPGSA,A,1,,,,,,,,,,,,,,,*1E
  $GPGSV,1,1,01,11,,,33*78
  $GPRMC,235945.020,V,,,,,0.00,0.00,050180,,,N*4F
  $GPVTG,0.00,T,,M,0.00,N,0.00,K,N*32
  $GPGGA,235946.020,,,,,0,0,,,M,,M,,*45
  $GPGLL,,,,,235946.020,V,N*77
  $GPGSA,A,1,,,,,,,,,,,,,,,*1E
  $GPGSV,1,1,02,22,,,36,11,,,33*7E
  $GPRMC,235946.020,V,,,,,0.00,0.00,050180,,,N*4C

  Timeout - No GPS Fix 5s
  Wait GPS Fix 5 seconds

  That printout is from a Mediatek GPS, the Ublox ones are similar. The data from the GPS is fed into the
  TinyGPS++ library and if there is no fix a message is printed on the serial monitor.

  When the program detects that the GPS has a fix, it prints the Latitude, Longitude, Altitude, Speed, Number
  of satellites in use, the HDOP value, time and date to the serial monitor. If the I2C OLED display is
  attached that is updated as well. The OLED display is assumed to be on I2C address 0x3C. Connect the GPS RX
  pin to XIAO A6 (TX) and GPS TX pin to XIAO A7 (TX).

  Serial monitor baud rate is set at 115200.

*******************************************************************************************************/

#include <TinyGPS++.h>                             //get library here > http://arduiniana.org/libraries/tinygpsplus/
TinyGPSPlus gps;                                   //create the TinyGPS++ object

#include <U8x8lib.h>                                      //get library here >  https://github.com/olikraus/u8g2 
//U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);  //use this line for standard 0.96" SSD1306
U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);     //use this line for 1.3" OLED often sold as 1.3" SSD1306
#define DEFAULTFONT u8x8_font_chroma48medium8_r           //font used by U8X8 Library

float GPSLat;                                      //Latitude from GPS
float GPSLon;                                      //Longitude from GPS
float GPSAlt;                                      //Altitude from GPS
uint8_t GPSSats;                                   //number of GPS satellites in use
uint32_t GPSHdop;                                  //HDOP from GPS
float GPSSpeed;                                    //Speed of GPS, mph

uint8_t hours, mins, secs, day, month;
uint16_t year;
uint32_t startGetFixmS;
uint32_t endFixmS;


void loop()
{
  if (gpsWaitFix(5))
  {
    Serial.println();
    Serial.println();
    Serial.print(F("Fix time "));
    Serial.print(endFixmS - startGetFixmS);
    Serial.println(F("mS"));

    GPSLat = gps.location.lat();
    GPSLon = gps.location.lng();
    GPSAlt = gps.altitude.meters();
    GPSSats = gps.satellites.value();
    GPSHdop = gps.hdop.value();
    GPSSpeed = gps.speed.mph();

    hours = gps.time.hour();
    mins = gps.time.minute();
    secs = gps.time.second();
    day = gps.date.day();
    month = gps.date.month();
    year = gps.date.year();

    printGPSfix();               //print GPS data to serial monitor
    displayscreen1();            //print GPS data on display
    startGetFixmS = millis();    //have a fix, next thing that happens is checking for a fix, so restart timer
  }
  else
  {
    disp.clearLine(0);
    disp.setCursor(0, 0);
    disp.print(F("No GPS Fix "));
    disp.print( (millis() - startGetFixmS) / 1000 );
    Serial.println();
    Serial.println();
    Serial.print(F("Timeout - No GPS Fix "));
    Serial.print( (millis() - startGetFixmS) / 1000 );
    Serial.println(F("s"));
  }
}


bool gpsWaitFix(uint16_t waitSecs)
{
  //waits a specified number of seconds for a fix, returns true for updated fix

  uint32_t startmS, waitmS;
  uint8_t GPSchar;

  Serial.print(F("Wait GPS Fix "));
  Serial.print(waitSecs);
  Serial.println(F(" seconds"));

  waitmS = waitSecs * 1000;                               //convert seconds wait into mS
  startmS = millis();

  while ( (uint32_t) (millis() - startmS) < waitmS)       //allows for millis() overflow
  {
    if (Serial1.available() > 0)
    {
      GPSchar = Serial1.read();
      gps.encode(GPSchar);
      Serial.write(GPSchar);
    }

    if (gps.location.isUpdated() && gps.altitude.isUpdated() && gps.date.isUpdated())
    {
      endFixmS = millis();                                //record the time when we got a GPS fix
      return true;
    }
  }
  return false;
}


void printGPSfix()
{
  float tempfloat;

  Serial.print(F("New GPS Fix "));

  tempfloat = ( (float) GPSHdop / 100);

  Serial.print(F("Latitude,"));
  Serial.print(GPSLat, 6);
  Serial.print(F(",Longitude,"));
  Serial.print(GPSLon, 6);
  Serial.print(F(",Altitude,"));
  Serial.print(GPSAlt, 1);
  Serial.print(F("m,Speed,"));
  Serial.print(GPSSpeed, 1);
  Serial.print(F("mph,Sats,"));
  Serial.print(GPSSats);
  Serial.print(F(",HDOP,"));
  Serial.print(tempfloat, 2);
  Serial.print(F(",Time,"));

  if (hours < 10)
  {
    Serial.print(F("0"));
  }

  Serial.print(hours);
  Serial.print(F(":"));

  if (mins < 10)
  {
    Serial.print(F("0"));
  }

  Serial.print(mins);
  Serial.print(F(":"));

  if (secs < 10)
  {
    Serial.print(F("0"));
  }

  Serial.print(secs);
  Serial.print(F(",Date,"));

  Serial.print(day);
  Serial.print(F("/"));
  Serial.print(month);
  Serial.print(F("/"));
  Serial.print(year);

  Serial.println();
  Serial.println();
}


void displayscreen1()
{
  //show GPS data on display
  float tempfloat;
  tempfloat = ( (float) GPSHdop / 100);

  disp.clearLine(0);
  disp.setCursor(0, 0);
  disp.print(GPSLat, 6);
  disp.clearLine(1);
  disp.setCursor(0, 1);
  disp.print(GPSLon, 6);
  disp.clearLine(2);
  disp.setCursor(0, 2);
  disp.print(GPSAlt, 0);
  disp.print(F("m"));
  disp.clearLine(3);
  disp.setCursor(0, 3);
  disp.print(GPSSpeed, 0);
  disp.print(F("mph"));
  disp.clearLine(4);
  disp.setCursor(0, 4);
  disp.print(F("Sats "));
  disp.print(GPSSats);
  disp.clearLine(5);
  disp.setCursor(0, 5);
  disp.print(F("HDOP "));
  disp.print(tempfloat);
  disp.clearLine(6);
  disp.setCursor(0, 6);

  if (hours < 10)
  {
    disp.print(F("0"));
  }

  disp.print(hours);
  disp.print(F(":"));

  if (mins < 10)
  {
    disp.print(F("0"));
  }

  disp.print(mins);
  disp.print(F(":"));

  if (secs < 10)
  {
    disp.print(F("0"));
  }

  disp.print(secs);
  disp.print(F("  "));

  disp.clearLine(7);
  disp.setCursor(0, 7);

  disp.print(day);
  disp.print(F("/"));
  disp.print(month);
  disp.print(F("/"));
  disp.print(year);
}


void setup()
{
  Serial1.begin(9600);                   //GPS RX pin to XIAO A6 (TX) and GPS TX pin to XIAO A7 (TX).

  Serial.begin(115200);
  Serial.println();
  Serial.print(F("10_GPS_Location_Checker_With_Display"));
  Serial.println();

  disp.begin();
  disp.setFont(DEFAULTFONT);
  disp.clear();
  disp.setCursor(0, 0);
  disp.print(F("Display Ready"));

  Serial.println();
  startGetFixmS = millis();
}
