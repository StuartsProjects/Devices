/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This program tests the sleep mode and register retention of the LoRa device in sleep
  mode, the program is for a Seeeduino XIAO. The LoRa settings to use are specified in
  the 'Settings.h' file.

  The program sets up the LoRa device then goes into deep sleep mode. When the switch is activated the
  board wakes up, transmits a LoRa packet and turns on the board LED. The processor stays awake for 15
  seconds, turns off the LED and goes into sleep mode. In sleep mode the current used is 20uA
  after the green power LED has been removed.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/
#include "ArduinoLowPower.h"                    //get library here > https://github.com/arduino-libraries/ArduinoLowPower

#include <SPI.h>
#include <SX127XLT.h>                           //get library here > https://github.com/StuartsProjects/SX12XX-LoRa   
#include "Settings.h"

SX127XLT LoRa;

uint8_t TXPacketL;


void loop()
{

  if (Send_Test_Packet())
  {
    packet_is_OK();
  }
  else
  {
    packet_is_Error();
  }
  Serial.println();

  delay(15000);                                  //allow time to re-load another program if required

  LoRa.setSleep(CONFIGURATION_RETENTION);        //preserve register settings in sleep.
  Serial.println(F("Sleeping zzzzz...."));
  Serial.println();
  Serial.flush();
  delay(2000);

  digitalWrite(LED1, HIGH);                      //LED off when in sleep
  LowPower.deepSleep();

  led_Flash(5, 125);                             //five quick LED flashes to indicate program re-start

  Serial.println(F("Awake !"));
  Serial.flush();
  LoRa.wake();
}


void wakeUp()
{
  //handler for the interrupt
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
  LoRa.printIrqStatus();
  digitalWrite(LED1, LOW);                        //this leaves the LED on slightly longer for a packet error
}


bool Send_Test_Packet()
{
  uint8_t bufffersize;
  uint8_t buff[] = "Gate Open";

  digitalWrite(LED1, LOW);                      //LED on

  LoRa.printASCIIPacket( (uint8_t*) buff, bufffersize);
  Serial.println();

  if (LoRa.transmit( (uint8_t*) buff, sizeof(buff), 10000, TXpower, WAIT_TX))
  {
    digitalWrite(LED1, HIGH);                   //LED off
    return true;
  }
  else
  {
    digitalWrite(LED1, HIGH);                   //LED off
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
  pinMode(SWITCH1, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);                         //setup board LED pin as output
  digitalWrite(LED1, HIGH);                      //LED off
  led_Flash(2, 125);                             //2 LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  SPI.begin();

  while (!LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
    Serial.println(F("ERROR - No LoRa device responding"));
    led_Flash(10, 25);                           //10 fast LED flashes to indicate LoRa device not responding
  }

  Serial.println(F("LoRa device is responding"));
  led_Flash(2, 125);                             //2 LED flashes to indicate LoRa device is responding

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

  LowPower.attachInterruptWakeup(SWITCH1, wakeUp, CHANGE);          //setup interrupt for wake up

  Serial.println(F("Transmitter ready"));
}
