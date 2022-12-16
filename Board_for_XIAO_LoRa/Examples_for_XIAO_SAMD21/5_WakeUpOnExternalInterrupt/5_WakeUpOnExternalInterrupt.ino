/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This program puts an XIAO into deep sleep mode ready to be woken up by an external
  switch. In this example connect A6 to GND to wake the XIAO up from sleep.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/
#include "ArduinoLowPower.h"                    //get library here > https://github.com/arduino-libraries/ArduinoLowPower

#define LED1 13                                 //on board LED, yellow/orange 
#define SWITCH1 A4                              //switch pin, used to wake processor up


#include <SPI.h>
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa   
#include "Settings.h"


void loop()
{
  Serial.println(F("Sleeping zzzzz...."));
  Serial.println();
  Serial.flush();
  delay(1000);
  digitalWrite(LED1, HIGH);                    //make sure LED off when in sleep

  LowPower.deepSleep();

  led_Flash(4, 125);                           //4 LED flashes to indicate program re-start

  Serial.println(F("Awake !"));
  Serial.flush();
  led_Flash(15, 500);                          //15 1 second LED flashes to indicate program awake
}


void wakeUp()
{
  //handler for the interrupt
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;
  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);
    delay(delaymS);
    digitalWrite(LED1, HIGH);
    delay(delaymS);
  }
}


void setup()
{
  pinMode(SWITCH1, INPUT_PULLDOWN);
  pinMode(LED1, OUTPUT);                         //setup board LED pin as output
  digitalWrite(LED1, HIGH);                      //LED off
  led_Flash(2, 125);                             //2 LED flashes to indicate program start
  delay(2000);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  LowPower.attachInterruptWakeup(SWITCH1, wakeUp, CHANGE);          //setup interrupt for wake up
}
