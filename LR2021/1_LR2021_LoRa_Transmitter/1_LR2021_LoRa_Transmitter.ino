/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  This is a program that uses the LR2021 to transmit packets. The packet is in ASCII and the count of packets
  sent is in the packet. 
  
  The file defining the pins for the used microcontroller board such as SEEED_XIAO_Pins.h contains the pin 
  definitions used for the LoRa module. You need to connect the LR2021 up to the XIAO SPI pins also. 

  Sample serial port output;

  Transmitted > LoRa00010,Power,10dBm,Length,10,Packets,10,Errors,0
  Transmitted > LoRa00011,Power,10dBm,Length,10,Packets,11,Errors,0
  Transmitted > LoRa00012,Power,10dBm,Length,10,Packets,12,Errors,0

*******************************************************************************************************/

/*
  For default module settings, see the library wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#lr2021---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the LR2021 library
#include <RadioLib.h>
#include "Settings.h"

#include "SEEED_XIAO_Pins.h"  //include if processor board is XIAO
//#include "Arduino_DUE_Pins.h"  //include if processor board is Arduino DUE

LR2021 radio = new Module(LORA_NSS, LORA_DIO, LORA_RESET, LORA_BUSY);

uint8_t TXbuffer[10];    //buffer to hold packet to send
uint8_t TXPacketL;       //stores length of packet sent
uint32_t TXpacketCount;  //keep count of packets sent
uint32_t TXerrors;       //keep count of any packets with transmit errors
int16_t state;           //reported state of radio.functions


void loop() {

  digitalWrite(LED1, LEDON);
  TXpacketCount++;
  TXbuffer[0] = 'L';
  TXbuffer[1] = 'o';
  TXbuffer[2] = 'R';
  TXbuffer[3] = 'a';
  TXbuffer[4] = TXpacketCount / 10000 + '0';
  TXbuffer[5] = ((TXpacketCount % 10000) / 1000) + '0';
  TXbuffer[6] = ((TXpacketCount % 1000) / 100) + '0';
  TXbuffer[7] = ((TXpacketCount % 100) / 10) + '0';
  TXbuffer[8] = TXpacketCount % 10 + '0';
  TXbuffer[9] = 0;  //ensure null at end

  TXPacketL = sizeof(TXbuffer);  //set TXPacketL to length of array
  digitalWrite(LED1, LEDON);
  int16_t state = radio.transmit(TXbuffer, sizeof(TXbuffer));

  if (state == RADIOLIB_ERR_NONE) {
    packet_is_OK();
  } else {
    packet_is_Error();
  }

  delay(250);  //delay here so you can see LED flash during transmit
  digitalWrite(LED1, LEDOFF);
  delay(1000);  //have a delay between packets
}


void packet_is_OK() {
  Serial.print(F(" Transmitted > "));
  Serial.print((char*)TXbuffer);
  Serial.print(F(",Power,"));
  Serial.print(TXPower);  //print the transmit power defined
  Serial.print(F("dBm"));
  Serial.print(F(",Length,"));
  Serial.print(TXPacketL);
  Serial.print(F(",Packets,"));
  Serial.print(TXpacketCount);
  Serial.print(F(",Errors,"));
  Serial.print(TXerrors);
  Serial.println();
}


void packet_is_Error() {
  TXerrors++;
  Serial.print(F(" PacketError"));
  Serial.print(F(",Length,"));
  Serial.print(TXPacketL);
  Serial.print(F(",Packets,"));
  Serial.print(TXpacketCount);
  Serial.print(F(",Errors,"));
  Serial.print(TXerrors);
  Serial.print(F(",State,"));
  Serial.print(state, HEX);
  Serial.println();
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, LEDON);
    delay(delaymS);
    digitalWrite(LED1, LEDOFF);
    delay(delaymS);
  }
}

void print_Device_Settings() {
  Serial.println(F("LoRa Device Settings"));

  Serial.print(F("Frequency "));
  Serial.print(Frequency, 3);
  Serial.println(F("Khz"));

  Serial.print(F("Bandwith "));
  Serial.print(Bandwidth, 3);
  Serial.println(F("Khz"));

  Serial.print(F("SpreadingFactor "));
  Serial.println(SpreadingFactor);

  Serial.print(F("CodeRate 4:"));
  Serial.println(CodeRate);

  Serial.print(F("LoRaSyncWord 0x"));
  Serial.println(LoRaSyncWord, HEX);

  Serial.print(F("PreambleLength "));
  Serial.println(PreambleLength);

  Serial.print(F("TXPower "));
  Serial.print(TXPower);
  Serial.println(F("dBm"));

  Serial.print(F("TCXOVoltage "));
  Serial.print(TCXOVoltage, 1);
  Serial.println(F("V"));

  Serial.print(F("LORA_RESET pin "));
  Serial.println(LORA_RESET);

  Serial.print(F("LORA_DIO pin "));
  Serial.println(LORA_DIO);

  Serial.print(F("LORA_BUSY pin "));
  Serial.println(LORA_BUSY);

  Serial.print(F("LORA_NSS pin "));
  Serial.println(LORA_NSS);

  Serial.print(F("IRQDIONUM "));
  Serial.println(IRQDIONUM);
}


void setup() {
  pinMode(LED1, OUTPUT);     //initialize digital pin LED1 as an output.
  digitalWrite(LED1, HIGH);  //LED on

  Serial.begin(115200);

  if (!Serial) {};

  led_Flash(10, 250);

  Serial.println();
  Serial.println(__FILE__);

  // LR2021 allows to use any of the DIO pins as the interrupt
  // as an example, we set DIO11 to be the IRQ
  // this has to be done prior to calling begin()!
  radio.irqDioNum = IRQDIONUM;

  // also before calling begin(), correct crystal has to be selected
  // some LR2021 have a TCXO which needs 1.6V reference
  // set to 0 if your radio has an XTAL
  radio.tcxoVoltage = TCXOVoltage;

  Serial.print(F("Initializing Module for LoRa... "));
  ConfigLoRa_t config;
  config.frequency = Frequency;
  config.bandwidth = Bandwidth;
  config.spreadingFactor = SpreadingFactor;
  config.codingRate = CodeRate;
  config.syncWord = LoRaSyncWord;
  config.power = TXPower;
  config.preambleLength = PreambleLength;
  int state = radio.begin(config);

  //Alternative begin, listed in API but now depracated apparently
  //begin (float freq = 868.0, float bw = 125.0, uint8_t sf = 7, uint8_t cr = 5, uint8_t syncWord = RADIOLIB_LR2021_LORA_SYNC_WORD_PRIVATE, int8_t power = 10, uint16_t preambleLength = 8, float tcxoVoltage = 1.60;
  //int state = radio.begin(Frequency, Bandwidth, SpreadingFactor, CodeRate, SyncWord, TXPower, PreambleLength, TCXOVoltage);
  //int state = radio.begin(868.0, 125.0, 7, 5, 0x12, 10, 8, 1.6);

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    while (1) {
      led_Flash(5, 25);
      Serial.println(F("No device responding, code "));
      Serial.println(state, HEX);
    };
  }


  print_Device_Settings();
  Serial.println();
}
