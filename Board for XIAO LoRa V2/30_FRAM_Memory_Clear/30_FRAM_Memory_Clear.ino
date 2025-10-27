/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 17/2/23

  This program is supplied as is, it is up to the user of the program to decide if the program is suitable
  for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program tests clears an MB85RC16PNF or FM24CL64 FRAM from locations 0x00 to
  0x1FF that are used by the TTN sensor node program.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

//#include "FRAM_FM24CL64.h"            //library file for FM24CL64 FRAM, 64kbit, 8kbyte, I2C addresse 0x50 
#include "FRAM_MB85RC16PNF.h"           //library file for MB85RC16PNF FRAM, 16kbit, 2kbyte, I2C addresses 0x50 to 0x57

#define LED1 13                         //on board LED is yellow
#include <Wire.h>
#include "I2C_Scanner.h"
#define FillValue 0x00                  //value used to fill (clear) memory   

int16_t Memory_Address = 0x50;          //default start I2C address of MB85RC16PNF and FM24CL64 FRAM


void loop()
{
  Serial.println(F("Printing memory"));
  printMemory(0, 0x1FF);
  Serial.println();
  Serial.flush();

  Serial.println(F("Clearing memory"));

  fillMemory(0, 0x1FF, 0xFF);

  Serial.println(F("Printing memory"));
  printMemory(0, 0x1FF);
  Serial.println();
  Serial.flush();

  Serial.println(F("Memory cleared"));
  while(1);
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                    //board LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                     //board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                          //setup board LED pin as output
  led_Flash(10, 500);                             //10 LED flashes to indicate program start

  digitalWrite(LED1, LOW);                        //LED on

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("30_FRAM_Memory_Clear"));
  Serial.println();

  digitalWrite(LED1, HIGH);                       //LED off, means I2C scan did not lock up

  setup_I2CScan();
  run_I2CScan();
  delay(2000);

  memoryStart(Memory_Address);                    //optional command to start memory with I2C address, if required
}
