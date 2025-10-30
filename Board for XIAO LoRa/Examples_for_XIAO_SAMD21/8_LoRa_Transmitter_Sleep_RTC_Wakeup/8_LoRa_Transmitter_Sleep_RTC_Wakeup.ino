/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This program is for the Seeeduino XIAO. The program is woken up by the RTC alarm
  every minute, flashes the LED for one minute, sends the LoRa test message and goes back to sleep.

  Sleep current using this mode of RTC wakeup was 16uA with the XIAO power LED removed.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <RTCZero.h>                               //get library here > https://github.com/arduino-libraries/RTCZero  
#include "ArduinoLowPower.h"                       //get library here > https://github.com/arduino-libraries/ArduinoLowPower

RTCZero rtc;                                       //Create an RTCZero object

#include <SPI.h>
#include <SX127XLT.h>                              //get library here > https://github.com/StuartsProjects/SX12XX-LoRa  
#include "Settings.h"

SX127XLT LoRa;

uint8_t TXPacketL;
bool alarm = false;


void loop()
{
  led_Flash(60, 500);                              //1 second LED flashes for a minute

  if (Send_Test_Packet())
  {
    packet_is_OK();
  }
  else
  {
    packet_is_Error();
  }

  Serial.println();
  LoRa.setSleep(CONFIGURATION_RETENTION);          //preserve LoRa register settings in sleep.

  pinMode(A6, INPUT_PULLUP);
  pinMode(A7, INPUT_PULLUP);
  pinMode(A0, INPUT_PULLUP);
  pinMode(A9, INPUT_PULLUP);

  rtc.setTime(0, 0, 0);
  rtc.setAlarmTime(0, 1, 0);                       //set alarm for 1 minute
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
  LoRa.wake();
}


void alarmMatch()
{
  alarm = true;
}


void packet_is_OK()
{
  Serial.print(F(" "));
  Serial.print(TXPacketL);
  Serial.print(F(" Bytes SentOK"));
}


void packet_is_Error()
{
  uint16_t IRQStatus;
  IRQStatus = LoRa.readIrqStatus();                 //get the IRQ status
  Serial.print(F("SendError,"));
  Serial.print(F("Length,"));
  Serial.print(TXPacketL);
  Serial.print(F(",IRQreg,"));
  Serial.print(IRQStatus, HEX);
}


bool Send_Test_Packet()
{
  uint8_t bufffersize;
  uint8_t buff[] = "25C";

  if (LoRa.transmit( (uint8_t*) buff, sizeof(buff), 10000, TXpower, WAIT_TX))
  {
    return true;
  }
  else
  {
    return false;
  }
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
  delay(2000);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  SPI.begin();

  while (!LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("ERROR - No LoRa device responding"));
    led_Flash(10, 25);                            //10 fast LED flashes to indicate LoRa device not responding
  }

  Serial.println(F("LoRa device is responding"));
  led_Flash(2, 125);                             //2 LED flashes to indicate LoRa device is responding

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  rtc.begin();                                   //initialize RTC 24H format
  rtc.setDate(26, 9, 22);
}
