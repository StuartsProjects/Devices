/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/**************************************************************************************************************
  This is a program that uses the LR2021 to transmit packets. The packet is in ASCII and the count of packets
  sent is in the packet. 
  
  The file defining the pins for the used microcontroller board such as SEEED_XIAO_Board.h contains the pin 
  definitions used for the LoRa module. You need to connect the LR2021 up to the SPI pins also. 

  Sample serial port output;

  Transmitted > FLRC00021,Power,10dBm,Length,10,Packets,21,Errors,0
  Transmitted > FLRC00022,Power,10dBm,Length,10,Packets,22,Errors,0
  Transmitted > FLRC00023,Power,10dBm,Length,10,Packets,23,Errors,0

**************************************************************************************************************/

/*
  For default module settings, see the library wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#lr2021---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the LR2021 library
#include <RadioLib.h>
#include "Settings.h"

#include "SEEED_XIAO_Board.h"  //include if processor board is XIAO
//#include "Arduino_DUE_Board.h"  //include if processor board is Arduino DUE

LR2021 radio = new Module(LORA_NSS, LORA_DIO, LORA_RESET, LORA_BUSY);

uint8_t TXbuffer[10];    //buffer to hold packet to send
uint8_t TXPacketL;       //stores length of packet sent
uint32_t TXpacketCount;  //keep count of packets sent
uint32_t TXerrors;       //keep count of any packets with transmit errors
int16_t state;           //reported state of radio.functions
bool DISPLAYOK = true;

#ifdef USE_OLED128X64
//libraries for I2C SH1106 OLED
#include <Wire.h>
#include <U8x8lib.h>                                   //get library here >  https://github.com/olikraus/u8g2
U8X8_SH1106_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE);  //use this line for 1.3" OLED often sold as 1.3" SSD1306
//U8X8_SSD1306_128X64_NONAME_HW_I2C disp(U8X8_PIN_NONE); //use this line for 0.96" SSD1306
#define DEFAULTFONT u8x8_font_chroma48medium8_r  //font for U8X8 Library
#include "OLED128X64.h"                          //library of display functions
#endif

#ifdef USE_LCD20X4
//libraries for I2C LCD via PCF8574 interface, HD44780 type
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>           //get the library here > https://github.com/mathertel/LiquidCrystal_PCF8574
LiquidCrystal_PCF8574 disp(DISPLAYADDRESS);  //set the LCD address to address provided in Settings.h
#include "LCD20X4.h"                         //library of display functions
#endif


void loop() {

  digitalWrite(LED1, LEDON);
  TXpacketCount++;
  TXbuffer[0] = 'F';
  TXbuffer[1] = 'L';
  TXbuffer[2] = 'R';
  TXbuffer[3] = 'C';
  TXbuffer[4] = TXpacketCount / 10000 + '0';
  TXbuffer[5] = ((TXpacketCount % 10000) / 1000) + '0';
  TXbuffer[6] = ((TXpacketCount % 1000) / 100) + '0';
  TXbuffer[7] = ((TXpacketCount % 100) / 10) + '0';
  TXbuffer[8] = TXpacketCount % 10 + '0';
  TXbuffer[9] = 0;  //ensure null at end

  TXPacketL = sizeof(TXbuffer);  //set TXPacketL to length of array
  digitalWrite(LED1, LEDON);
  state = radio.transmit(TXbuffer, sizeof(TXbuffer));

  if (state == RADIOLIB_ERR_NONE) {
    packet_is_OK(state);
  } else {
    packet_is_Error(state);
  }

  delay(250);  //delay here so you can see LED flash during transmit
  digitalWrite(LED1, LEDOFF);
  delay(1000);  //have a delay between packets
}


void packet_is_OK(uint16_t state) {
  mySerial.print(F("Transmitted > "));
  mySerial.print((char*)TXbuffer);
  mySerial.print(F(",Power,"));
  mySerial.print(TXPower);  //print the transmit power defined
  mySerial.print(F("dBm"));
  mySerial.print(F(",Length,"));
  mySerial.print(TXPacketL);
  mySerial.print(F(",Packets,"));
  mySerial.print(TXpacketCount);
  mySerial.print(F(",Errors,"));
  mySerial.print(TXerrors);
  mySerial.println();
  if (DISPLAYOK) {
    display_TXpacket(TXbuffer, TXPacketL, TXpacketCount, TXerrors, TXPower, state);
  }
}


void packet_is_Error(uint16_t state) {
  TXerrors++;
  mySerial.print(F(" PacketError"));
  mySerial.print(F(",Length,"));
  mySerial.print(TXPacketL);
  mySerial.print(F(",Packets,"));
  mySerial.print(TXpacketCount);
  mySerial.print(F(",Errors,"));
  mySerial.print(TXerrors);
  mySerial.print(F(",State,"));
  mySerial.print(state, HEX);
  mySerial.println();
  if (DISPLAYOK) {
    display_TXpacket(TXbuffer, TXPacketL, TXpacketCount, TXerrors, TXPower, state);
  }
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


void print_FLRCModemSettings() {
  mySerial.println(F("FLRC Modem Settings"));

  mySerial.print(F("Frequency "));
  mySerial.print(Frequency, 3);
  mySerial.println(F("Mhz"));

  mySerial.print(F("BandwidthBitRate "));
  mySerial.print(BandwidthBitRate);
  mySerial.println(F("Kbps"));

  mySerial.print(F("CodingRate "));

  switch (CodingRate) {

    case RADIOLIB_LR2021_FLRC_CR_1_2:
      mySerial.println(F("1_2"));
      break;

    case RADIOLIB_LR2021_FLRC_CR_3_4:
      mySerial.println(F("3_4"));
      break;

    case RADIOLIB_LR2021_FLRC_CR_1_0:
      mySerial.println(F("1_0"));
      break;

    case RADIOLIB_LR2021_FLRC_CR_2_3:
      mySerial.println(F("2_3"));
      break;

    default:
      mySerial.println(F("Invalid"));
      break;
  }

  mySerial.print(F("TXPower "));
  mySerial.print(TXPower);
  mySerial.println(F("dBm"));

  mySerial.print(F("PreambleLength "));
  mySerial.println(PreambleLength);

  mySerial.print(F("DataShaping "));

  switch (DataShaping) {

    case RADIOLIB_SHAPING_0_3:
      mySerial.println(F("0_3"));
      break;

    case RADIOLIB_SHAPING_0_5:
      mySerial.println(F("0_5"));
      break;

    case RADIOLIB_SHAPING_0_7:
      mySerial.println(F("0_7"));
      break;

    case RADIOLIB_SHAPING_1_0:
      mySerial.println(F("1_0"));
      break;

    case RADIOLIB_SHAPING_NONE:
      mySerial.println(F("NONE"));
      break;

    default:
      mySerial.println(F("Invalid"));
      break;
  }
}


void print_Device_Settings() {
  mySerial.println(F("Device Settings"));

  mySerial.print(F("LORA_RESET pin "));
  mySerial.println(LORA_RESET);

  mySerial.print(F("LORA_DIO pin "));
  mySerial.println(LORA_DIO);

  mySerial.print(F("LORA_BUSY pin "));
  mySerial.println(LORA_BUSY);

  mySerial.print(F("LORA_NSS pin "));
  mySerial.println(LORA_NSS);

  mySerial.print(F("IRQDIONUM "));
  mySerial.println(IRQDIONUM);

  mySerial.print(F("TCXOVoltage "));
  mySerial.print(TCXOVoltage, 1);
  mySerial.println(F("V"));
}


void setup() {
  pinMode(LED1, OUTPUT);     //initialize digital pin LED1 as an output.
  digitalWrite(LED1, HIGH);  //LED on

  mySerial.begin(115200);

  if (!Serial) {};

  led_Flash(10, 250);

  mySerial.println();
  mySerial.println(__FILE__);

  // LR2021 allows to use any of the DIO pins as the interrupt
  // as an example, we set DIO11 to be the IRQ
  // this has to be done prior to calling begin()!
  radio.irqDioNum = IRQDIONUM;

  // also before calling begin(), correct crystal has to be selected
  // some LR2021 have a TCXO which needs 1.6V reference
  // set to 0 if your radio has an XTAL
  radio.tcxoVoltage = TCXOVoltage;

  mySerial.print(F("Initializing Module for FLRC ... "));
  ConfigFLRC_t config;
  config.frequency = Frequency;
  config.bitRate = BandwidthBitRate;
  config.codingRate = CodingRate;
  config.power = TXPower;
  config.preambleLength = PreambleLength;
  config.dataShaping = DataShaping;
  state = radio.beginFLRC(config);

  if (state == RADIOLIB_ERR_NONE) {
    mySerial.println(F("success!"));
  } else {
    while (1) {
      led_Flash(5, 25);
      mySerial.print(F("Error Initializing, state "));
      mySerial.println(state);
    };
  }

#ifdef USE_OLED128X64
  if (setup_Display(DISPLAYADDRESS)) {
    mySerial.println("OLED128X64 Display init OK");
    DISPLAYOK = true;
  } else {
    mySerial.println("OLED128X64 Display init Failed");
    DISPLAYOK = false;
  }
#endif

#ifdef USE_LCD20X4
  if (setup_Display(DISPLAYADDRESS)) {
    mySerial.println("LCD20X4 Display init OK");
    DISPLAYOK = true;
  } else {
    mySerial.println("LCD20X4 Display init Failed");
    DISPLAYOK = false;
  }
#endif

  mySerial.println();
  print_Device_Settings();
  mySerial.println();
  print_FLRCModemSettings();
  mySerial.println();


  if (DISPLAYOK) {
    display_FLRCModemSettings(Frequency, BandwidthBitRate, CodingRate, TXPower, PreambleLength, DataShaping);
    delay(2000);
  }
  mySerial.println(F("Transmitter ready"));
}
