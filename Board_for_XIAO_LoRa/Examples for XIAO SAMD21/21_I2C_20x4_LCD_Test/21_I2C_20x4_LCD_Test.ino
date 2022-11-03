/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 28/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/**************************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.
  
  Program operation - This is a test program for a 3.3V 20x4 LCD display with HD44870 controller driven via the
  PCF8563 I2C I\O expander. These are back packs that fit on the LCD display are readily available at low cost.

  First the program scans the I2C bus for devices, it should report a device found at 0x3F, this is the PCF8563.

  Then you should see this as the text on the LCD, assuming you have adjusted the contrast correctly.

  StuartsProjects
  I2C LCD Backpack
  0123456789ABCDEFGHIJ
  Last Line of Text

*****************************************************************************************************************/


#include <Wire.h>
#include <LiquidCrystal_I2C.h>                   //www.4tronix.co.uk/arduino/sketches/LiquidCrystal_V1.2.1.zip
#include "I2C_Scanner.h"

#define PCF8574Address 0x3F                      //address of PCF8574 can change, I have noted addresses of 0x27 and 0x3F

LiquidCrystal_I2C disp(PCF8574Address, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  //Set the LCD I2C address and pins used


void loop()
{
  screen1();
  delay(2000);
  disp.clear();
}


void screen1()
{
  disp.setCursor(0, 0);               //Start at character 4 on line 0
  disp.print("StuartsProjects");
  delay(500);
  disp.setCursor(0, 1);
  disp.print("I2C LCD Backpack");     //Print text on 2nd Line
  delay(500);
  disp.setCursor(0, 2);
  disp.print("0123456789ABCDEFGHIJ"); //Print 20 characters on 3rd line
  delay(500);
  disp.setCursor(0, 3);
  disp.print("Last Line of Text");
}


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();

  setup_I2CScan();
  run_I2CScan();

  disp.begin(20, 4);                   //initialize the lcd for 20 chars 4 lines, turn on backlight
  disp.backlight();                    //backlight on
}
