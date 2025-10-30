/*
  3X4-MPR121-Capacitive-Touch-Keypad-Module
  Modified on 10 Jan 2021
  by Amir Mohammad Shojaee @ Electropeak
  Home

  based on Adafruit Library Example
*/

/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 28/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/**************************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program operation - This is a test program for the MPR121 I2C keypad.

*****************************************************************************************************************/



#include <Wire.h>
#include "Adafruit_MPR121.h"                              //get library here > https://github.com/adafruit/Adafruit_MPR121

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

// You can have up to 4 keypads on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;


void loop()
{
  currtouched = cap.touched();                       // Get the currently touched pads

  for (uint8_t i = 0; i < 12; i++) {

    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) )  // it if *is* touched and *wasnt* touched before, alert!
    {
      Serial.print(i); Serial.print(" touched");
    }

    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) )  // if it *was* touched and now *isnt*, alert!
    {
      Serial.println("       released");
    }
  }

  lasttouched = currtouched;                           // reset our state

  return;                                              // comment out this line for detailed data from the sensor!

  Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x"); Serial.println(cap.touched(), HEX);  // debugging info
  Serial.print("Filt: ");

  for (uint8_t i = 0; i < 12; i++)
  {
    Serial.print(cap.filteredData(i)); Serial.print("\t");
  }

  Serial.println();
  Serial.print("Base: ");

  for (uint8_t i = 0; i < 12; i++)
  {
    Serial.print(cap.baselineData(i)); Serial.print("\t");
  }
  Serial.println();
  delay(100);                                         // put a delay so it isn't overwhelming
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  while (!Serial)
  {
    delay(10);
  }

  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  //Default address is 0x5A, if tied to 3.3V its 0x5B,
  //If tied to SDA its 0x5C and if SCL then 0x5D

  if (!cap.begin(0x5A))
  {
    Serial.println("MPR121 not found, check wiring?");
    led_Flash(10, 25);                            //10 fast LED flashes to indicate device not responding
  }

  Serial.println("MPR121 found");
}
