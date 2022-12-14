/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This program is for the Seeeduino XIAO. It flashes the on board LED for one minute,
  then sets the RTC alarm and goes into deep sleep until the alarm occurs 60 seconds later.

  Sleep current using this mode of RTC wakeup was 4.7uA.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <RTCZero.h>                            //get the library here > https://github.com/arduino-libraries/RTCZero  
#include "ArduinoLowPower.h"                    //get library here > https://github.com/arduino-libraries/ArduinoLowPower

RTCZero rtc;                                    //Create an RTCZero object

bool alarm = false;

#define LED1 13


void loop()
{
  led_Flash(60, 500);                           //1 second LED flashes for a minute

  rtc.setTime(0, 0, 0);
  rtc.setAlarmTime(0, 1, 0 );                  //set alarm for 1 minute
  rtc.attachInterrupt(alarmMatch);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  Serial.println(F("Waiting"));
  Serial.flush();
  delay(2000);

  LowPower.deepSleep();

  alarm = false;
  rtc.disableAlarm();
  Serial.println(F("Alarm !"));
  led_Flash(10, 50);
}


void alarmMatch()
{
  alarm = true;
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
  pinMode(LED1, OUTPUT);                         //setup board LED pin as output
  digitalWrite(LED1, HIGH);                      //LED off
  led_Flash(2, 125);                             //2 LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  rtc.begin();                                  //initialize RTC 24H format
  rtc.setDate(26, 9, 22);
}
