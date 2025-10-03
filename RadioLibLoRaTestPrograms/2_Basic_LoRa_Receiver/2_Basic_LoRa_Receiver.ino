/*
  RadioLib LR1121 and SX1262 Receiver

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

/*
  Program by Stuart Robinson - 26/09/25
  Contains variables and code used by transmitter and receiver programs
*/

/*******************************************************************************************************
  This is a basic LoRa packet receiver program, originally developed for the Lilygo T3S3 board. 

  Matching companion transmitter program is 1_Basic_LoRa_Transmitter.ino

  Set the board type to ESP32S3 Dev Module. 
    
  Uses the USB port on the T3S3 for Serial monitor printing, so set USB CDC on Boot: option to "Enabled"

  Using Bluetooth option requires version 3.0.0 + of Arduino ESP32 core, version 3.3.0 used in this example

  Issues:

*******************************************************************************************************/

#include "Settings.h"  //contains LoRa and program settings etc

// select the pins file for the board in use
#include "Lilygo_T3S3.h"  //pins for for Lilygo T3S3 with LR1121 or SX1262 module

#include <RadioLib.h>  //get library here > https://github.com/jgromes/RadioLib

#ifdef USE_LR1121
LR1121 radio = new Module(NSS_PIN, IRQ_PIN, RESET_PIN, BUSY_PIN);
const char selecteddevice[] = "LR1121";
#endif

#ifdef USE_SX1262
SX1262 radio = new Module(NSS_PIN, DIO1_PIN, RESET_PIN, BUSY_PIN);
const char selecteddevice[] = "SX1262";
#endif

#ifdef USE_SX1276
SX1276 radio = new Module(NSS_PIN, DIO0_PIN, RESET_PIN, DIO1_PIN);
const char selecteddevice[] = "SX1276";
#endif

// flag to indicate that a packet was sent
volatile bool transmittedFlag = false;

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// this function is called when a complete packet
// is transmitted\received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif
void setFlag(void) {
  // we sent a packet, set the flag
  transmittedFlag = true;

  // we got a packet, set the flag
  receivedFlag = true;
}

#ifdef USE_SD
#ifdef USE_ESP32
SPIClass SDCardSPI(HSPI);
#endif
#endif

#ifdef USE_DISPLAY
#include <U8g2lib.h>                                               //get library here > https://github.com/olikraus/U8g2_Arduino
U8G2_SSD1306_128X64_NONAME_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);  //for T3S3 display
//U8G2_SH1106_128X64_NONAME_F_HW_I2C disp(U8G2_R0, U8X8_PIN_NONE);   //1.3" version of OLED
#endif

uint8_t RXPacketL;     //stores length of received packet
uint32_t RXPacketsOK;  //count of packets received OK
float PacketRSSI;      //stores RSSI of received packet
float PacketSNR;       //stores signal to noise ratio (SNR) of received packet
uint8_t RXbuff[255];   //received packet stored here

uint8_t TXPacketL;     //stores length of transmitted packet
uint32_t TXPacketsOK;  //count of packets transmitted OK
uint8_t TXbuff[10];    //packet to send

uint32_t PacketCount = 0;  //count of packets processed
uint32_t PacketErrors;  //count of packets received\transmitted with errors

int16_t LoRastate = RADIOLIB_ERR_NONE;  //allow global use of LoRastate
float Voltage = 0;                      //some boards can read battery voltage

char FileName[] = "/Log0000.csv";  //base FileName for SD file loging

bool LORA_FOUND = false;  //set to true if LoRa device detected
bool OLED_FOUND = false;  //set to true if OLED detected
bool SD_Found = false;    //set to true if SD card detected

#ifdef USE_BLUETOOTH
#include "BlueTooth.h"
#endif

#include "LoRa_Test_Functions.h"  //collection of TX\RX functions for Serial.print, OLED display and SD card


void loop() {

  if (receivedFlag) {  //check if the receive flag is set
    digitalWrite(LED1, HIGH);

#ifdef USE_BUZZER
    digitalWrite(BUZZER_PIN, HIGH);
#endif

    receivedFlag = false;  //reset receive flag
    RXPacketL = radio.getPacketLength();
    PacketRSSI = radio.getRSSI();
    PacketSNR = radio.getSNR();

    LoRastate = radio.readData(RXbuff, RXPacketL);  //read received data into byte array

    if (LoRastate == RADIOLIB_ERR_NONE) {
      RXPacketsOK++;
    } else {
      PacketErrors++;
    }

    print_packet_RX_detail(LoRastate);
    Serial.println();

#ifdef USE_LILYGOT3S3
    Voltage = (analogReadMilliVolts(ADC_PIN) * 2) / 1000.0;
#endif

#ifdef USE_DISPLAY
    display_packet_RX_detail(LoRastate);
#endif

#ifdef USE_SD
    if (SD_Found) {
      log_packet_RX_SD(LoRastate);
    }
#endif

    digitalWrite(LED1, LOW);

#ifdef USE_BUZZER
    digitalWrite(BUZZER_PIN, LOW);
#endif

#ifdef USE_BLUETOOTH
    log_packetRXBluetooth(LoRastate);
#endif

    radio.startReceive();  //put LoRa module back to listen mode
  }
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;
  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, HIGH);
    delay(delaymS);
    digitalWrite(LED1, LOW);
    delay(delaymS);
  }
}


void setup() {
#ifdef USE_BUZZER
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
#endif

  pinMode(LED1, OUTPUT);
  led_Flash(10, 250);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

#ifdef USE_SD
  setup_SDLog(FileName);
  log_Setup_SD();
#endif

#ifdef USE_ESP32
  SPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN);
#else
  SPI.begin();
#endif

  // initialise LoRa module with default settings
  Serial.print(selecteddevice);
  Serial.print(F(" Initializing ... "));

  LoRastate = radio.begin();

  if (LoRastate == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
    led_Flash(2, 50);
    LORA_FOUND = true;
  } else {
    Serial.print(selecteddevice);
    Serial.print(F(" LoRa device failed, code "));
    Serial.println(LoRastate);
    LORA_FOUND = false;
  }

#ifdef USE_DISPLAY
  init_Display(OLEDAddress);
  delay(1500);
  display_Details();
  delay(1500);
#endif

  if (!LORA_FOUND) {
    while (true) {
      led_Flash(2, 50);
    }
  }

#ifndef USE_SX1276  //no library support for TCXO on SX1276
  radio.setTCXO(TCXOvolts);
#endif

#ifdef USE_SX1262
  // Some SX126x modules use DIO2 as RF switch. To enable
  // this feature, the following method can be used.
  // NOTE: As long as DIO2 is configured to control RF switch,
  //       it can't be used as interrupt pin!
  if (radio.setDio2AsRfSwitch() != RADIOLIB_ERR_NONE) {
    Serial.println(F("Failed to set DIO2 as RF switch!"));
    while (true)
      ;
  }
#endif

  // set the function that will be called
  // when new packet is received
  radio.setPacketReceivedAction(setFlag);

  // set the function that will be called
  // when packet transmission is finished
  // radio.setPacketSentAction(setFlag);

  radio.explicitHeader();

  if (radio.setFrequency(Frequency) == RADIOLIB_ERR_INVALID_FREQUENCY) {
    Serial.println(F("Selected frequency is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  if (radio.setBandwidth(Bandwidth) == RADIOLIB_ERR_INVALID_BANDWIDTH) {
    Serial.println(F("Selected bandwidth is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  if (radio.setSpreadingFactor(SpreadingFactor) == RADIOLIB_ERR_INVALID_SPREADING_FACTOR) {
    Serial.println(F("Selected spreading factor is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  if (radio.setCodingRate(CodeRate) == RADIOLIB_ERR_INVALID_CODING_RATE) {
    Serial.println(F("Selected coding rate is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  if (radio.setSyncWord(SyncWord) != RADIOLIB_ERR_NONE) {
    Serial.println(F("Unable to set sync word!"));
    while (true) {
      delay(10);
    }
  }

  if (radio.setPreambleLength(PreambleLength) == RADIOLIB_ERR_INVALID_PREAMBLE_LENGTH) {
    Serial.println(F("Selected preamble length is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

  // enables or disables CRC check of received packets.
  if (radio.setCRC(CRCon) == RADIOLIB_ERR_INVALID_CRC_CONFIGURATION) {
    Serial.println(F("Selected CRC is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }

#ifdef USE_LR1121
  if (radio.setOutputPower(TXpower, true) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println(F("Selected output power is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }
#endif

#ifdef USE_SX1262
  if (radio.setOutputPower(TXpower) == RADIOLIB_ERR_INVALID_OUTPUT_POWER) {
    Serial.println(F("Selected output power is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }
#endif


#ifdef USE_SX1262
  if (radio.setCurrentLimit(CurrentLimit) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
    Serial.println(F("Selected current limit is invalid for this module!"));
    while (true) {
      delay(10);
    }
  }
#endif


#ifdef USE_LR1121
  static const uint32_t rfswitch_dio_pins[] = {
    RADIOLIB_LR11X0_DIO5, RADIOLIB_LR11X0_DIO6,
    RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC
  };

  static const Module::RfSwitchMode_t rfswitch_table[] = {
    // mode                  DIO5  DIO6
    { LR11x0::MODE_STBY, { LOW, LOW } },
    { LR11x0::MODE_RX, { HIGH, LOW } },
    { LR11x0::MODE_TX, { LOW, HIGH } },
    { LR11x0::MODE_TX_HP, { LOW, HIGH } },
    { LR11x0::MODE_TX_HF, { LOW, LOW } },
    { LR11x0::MODE_GNSS, { LOW, LOW } },
    { LR11x0::MODE_WIFI, { LOW, LOW } },
    END_OF_MODE_TABLE,
  };
  radio.setRfSwitchTable(rfswitch_dio_pins, rfswitch_table);
#endif

  Serial.print(F("LoRa SyncWord 0x"));
  Serial.println(SyncWord, HEX);

  // start listening for LoRa packets
  Serial.print(F("Starting to listen ... "));
  LoRastate = radio.startReceive();
  if (LoRastate == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(LoRastate);
#ifdef USE_DISPLAY
    display_LoRa_error(LoRastate);
#endif
    while (true) {
      led_Flash(2, 20);
    }
  }

#ifdef USE_LILYGOT3S3
  Voltage = (analogReadMilliVolts(ADC_PIN) * 2) / 1000.0;
#endif

#ifdef USE_DISPLAY
  display_packet_RX_detail(0);
#endif

#ifdef USE_BLUETOOTH
  BLE_setup();
#endif

  print_Details();
  Serial.println(F("Listening"));
}
