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

#define LED1 13                                 //on board LED, yellow 
#define SWITCH1 A6                              //switch pin, used to wake processor up


void loop()
{
  Serial.println(F("Sleeping zzzzz...."));
  Serial.println();
  Serial.flush();
  delay(2000);
  digitalWrite(LED1, HIGH);                    //make sure LED off when in sleep

  LowPower.deepSleep();

  led_Flash(10, 50);                           //10 quick LED flashes to indicate program re-start

  Serial.println(F("Awake !"));
  Serial.flush();
  led_Flash(60, 500);                          //60 1 second LED flashes to indicate program awake
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
  pinMode(SWITCH1, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);

  led_Flash(2, 125);                                                //two quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("5_WakeUpOnExternalInterrupt Starting"));

  LowPower.attachInterruptWakeup(SWITCH1, wakeUp, CHANGE);          //setup interrupt for wake up
}
