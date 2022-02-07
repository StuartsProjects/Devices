/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/02/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a receiver program for an ESP32CAM used with the ESP32CAM Long Range Wireless
  Adapter Board that will recive pictures sent from another ESP32CAM that is using program
  3_ESP32CAM_Transmit_Picture. The received pictures are saved to the ESP32CAMs SD card.

  The image\array is then transfered to a PC running Tera Term via the YModem protocol.

  Progress and monitor messages are set to the Serial2 TX pin which is set at pin 33, the Red LED pin.
  A wire link needs to be added to connect the Serial2 output connector, CONA on the ESP32CAM Long Range
  Wireless Adapter PCB.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>
#include "FS.h"                            //SD Card ESP32
#include "SD_MMC.h"                        //SD Card ESP32
#include "soc/soc.h"                       //disable brownout problems
#include "soc/rtc_cntl_reg.h"              //disable brownout problems
#include "driver/rtc_io.h"
#include <SX127XLT.h>                      //SX12XX-LoRa library
#include <ProgramLT_Definitions.h>         //part of SX12XX-LoRa library
#include "Settings.h"                      //LoRa settings etc.

#define ENABLEPCTRANSFER                   //enable this define to transfer array\image to PC via Ymodem 
#define ENABLEMONITOR                      //enable this define to monitor data transfer information, needed for ARtransferIRQ.h
#define ENABLEARRAYCRC                     //enable this define to check and print CRC of sent array                   
#define PRINTSEGMENTNUM                    //enable this define to print segment numbers during data transfer
//#define DISABLEPAYLOADCRC                //enable this define if you want to disable payload CRC checking
//#define DEBUG                            //enable more detail of transfer progress


SX127XLT LoRa;                             //create an SX127XLT library instance called LoRa
#include <ARtransferIRQ.h>

uint8_t *PSRAMptr;                         //create a global pointer to the array to send, so all functions have access
bool SDOK;
bool savedtoSDOK;
uint32_t arraylength;
uint32_t bytestransfered;

#include "YModemArray.h"                   //YModem transferring array functions, local file


void loop()
{
  SDOK = false;
  Serial2.println(F("LoRa file transfer receiver ready"));
  setupLoRaDevice();

  //if there is a successful array transfer the returned length > 0
  arraylength = LocalARreceiveArray(PSRAMptr, sizeof(ARDTarraysize), ReceiveTimeoutmS);

  SPI.end();

  if (arraylength)
  {
    Serial2.print(F("Returned picture length "));
    Serial2.println(arraylength);
    if (initMicroSDCard())
    {
      SDOK = true;
      Serial2.println("SD Card OK");
      Serial2.println(F("Save picture to SD card"));
      fs::FS &fs = SD_MMC;                            //save picture to microSD card
      File file = fs.open(ARDTfilenamebuff, FILE_WRITE);
      if (!file)
      {
        Serial2.println("*********************************************");
        Serial2.println("ERROR Failed to open SD file in writing mode");
        Serial2.println("*********************************************");
        savedtoSDOK = false;
      }
      else
      {
        file.write(PSRAMptr, arraylength); // pointer to array and length
        Serial2.print(ARDTfilenamebuff);
        Serial2.println(" Saved to SD");
        savedtoSDOK = true;
      }
      file.close();
      SD_MMC.end();
    }
    else
    {
      Serial2.println("No SD available");
    }
#ifdef ENABLEPCTRANSFER
    YmodemTransfer();
#endif
  }
  else
  {
    Serial2.println(F("Error receiving picture"));
    if (ARDTArrayTimeout)
    {
      Serial2.println(F("Timeout receiving picture"));
    }
  }
  Serial2.println();
}


bool YmodemTransfer()
{
  Serial2.println(F("YModem file transfer starting"));
  Serial2.flush();
  led_Flash(1, 1000);
  bytestransfered = yModemSend(ARDTfilenamebuff, PSRAMptr, arraylength, 1, 1);
  Serial2.flush();
  led_Flash(1, 1000);

  if (bytestransfered == arraylength)
  {
    Serial2.println(F("YModem transfer OK"));
  }
  else
  {
    Serial2.println(F("YModem transfer FAILED"));
    return false;
  }
  Serial2.println();
  Serial2.flush();
  return true;
}


uint32_t LocalARreceiveArray(uint8_t *ptrarray, uint32_t length, uint32_t receivetimeout)
{
  //returns 0 if no ARDTArrayEnded set, returns length of array if received
  uint32_t startmS = millis();

  ptrARreceivearray = ptrarray;                        //set global pointer to array pointer passed
  ARArrayLength = length;
  ARDTArrayTimeout = false;
  ARDTArrayEnded = false;
  ARDTDestinationArrayLength = 0;

  do
  {
    if (LocalARreceivePacketDT())
    {
      startmS = millis();
    }

    if (ARDTArrayEnded)                                    //has the end array transfer been received ?
    {
      return ARDTDestinationArrayLength;
    }
  }
  while (((uint32_t) (millis() - startmS) < receivetimeout ));


  if (ARDTArrayEnded)                                    //has the end array transfer been received ?
  {
    return ARDTDestinationArrayLength;
  }
  else
  {
    ARDTArrayTimeout = true;
    return 0;
  }
}


bool LocalARreceivePacketDT()
{
  //Receive data transfer packets

  ARRXPacketType = 0;
  ARRXPacketL = LoRa.receiveDTIRQ(ARDTheader, HeaderSizeMax, (uint8_t *) ARDTdata, DataSizeMax, NetworkID, RXtimeoutmS, WAIT_RX);

  if (ARDTLED >= 0)
  {
    digitalWrite(ARDTLED, HIGH);
  }

#ifdef ENABLEMONITOR
#ifdef DEBUG
  ARprintSeconds();
#endif
#endif
  if (ARRXPacketL > 0)
  {
    //if the LoRa.receiveDTIRQ() returns a value > 0 for ARRXPacketL then packet was received OK
    //then only action payload if destinationNode = thisNode
    ARreadHeaderDT();                             //get the basic header details into global variables ARRXPacketType etc
    LocalARprocessPacket(ARRXPacketType);         //process and act on the packet
    if (ARDTLED >= 0)
    {
      digitalWrite(ARDTLED, LOW);
    }
    return true;
  }
  else
  {
    //if the LoRa.receiveDT() function detects an error RXOK is 0

    uint16_t IRQStatus = LoRa.readIrqStatus();

    if (IRQStatus & IRQ_RX_TIMEOUT)
    {
      Monitorport.println(F("RX Timeout"));
    }
    else
    {
      ARRXErrors++;

#ifdef ENABLEMONITOR
      Monitorport.print(F("PacketError"));
      ARprintPacketDetails();
      ARprintReliableStatus();
      Monitorport.print(F("IRQreg,0x"));
      Monitorport.println(LoRa.readIrqStatus(), HEX);
      Monitorport.println();
#endif
    }

    if (ARDTLED >= 0)
    {
      digitalWrite(ARDTLED, LOW);
    }
    return false;
  }
}


bool LocalARprocessPacket(uint8_t packettype)
{
  //Decide what to do with an incoming packet

  if (packettype == DTSegmentWrite)
  {
    ARprocessSegmentWrite();
    return true;
  }

  if (packettype == DTArrayStart)
  {
    ARprocessArrayStart(ARDTdata, ARRXDataarrayL);       //ARDTdata contains the filename
    return true;
  }

  if (packettype == DTArrayEnd)
  {
    ARprocessArrayEnd();
    return true;
  }

  if (packettype == DTInfo)
  {
    LocalARprocessDTInfo();
    return true;
  }
  return true;
}


bool LocalARprocessDTInfo()
{
  // There is a info packet from transmitter

  ARRXFlags = ARDTheader[1];                          //read flags byte

#ifdef ENABLEMONITOR
  Monitorport.print(F("DTInfo packet received, flags byte 0x"));
  Monitorport.println(ARRXFlags, HEX);
#endif

  if bitRead(ARRXFlags, ARNoFileSave)
  {
#ifdef ENABLEMONITOR
    Monitorport.println();
    Monitorport.println(F("******************************"));
    Monitorport.println(F("Remote - No image saved to SD"));
    Monitorport.println(F("******************************"));
    Monitorport.println();
#endif
  }

  if bitRead(ARRXFlags, ARNoCamera)
  {
#ifdef ENABLEMONITOR
    Monitorport.println();
    Monitorport.println(F("*********************"));
    Monitorport.println(F("Remote camera failed"));
    Monitorport.println(F("*********************"));
    Monitorport.println();
#endif
  }

  ARDTheader[0] = DTInfoACK;                          //set ACK packet type
  delay(ACKdelaymS);

  if (ARDTLED >= 0)
  {
    digitalWrite(ARDTLED, HIGH);
  }

  LoRa.sendACKDTIRQ(ARDTheader, DTInfoHeaderL, TXpower);

  if (ARDTLED >= 0)
  {
    digitalWrite(ARDTLED, LOW);
  }
  return true;
}


bool setupLoRaDevice()
{
  SPI.begin(SCK, MISO, MOSI, NSS);

  if (LoRa.begin(NSS, NRESET, LORA_DEVICE))
  {
    Serial2.println(F("LoRa device found"));
  }
  else
  {
    Serial2.println(F("LoRa Device error"));
    return false;
  }

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);
  LoRa.setReliableConfig(NoReliableCRC);

  if (LoRa.getReliableConfig(NoReliableCRC))
  {
    Serial2.println(F("Payload CRC disabled"));
  }
  else
  {
    Serial2.println(F("Payload CRC enabled"));
  }
  return true;
}


bool initMicroSDCard()
{
  if (!SD_MMC.begin("/sdcard", true))               //use this line for 1 bit mode, pin 2 only, 4,12,13 not used
  {
    Serial2.println("*****************************");
    Serial2.println("ERROR - SD Card Mount Failed");
    Serial2.println("*****************************");
    return false;
  }

  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE)
  {
    Serial2.println("No SD Card found");
    return false;
  }
  return true;
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;
  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, HIGH);
    delay(delaymS);
    digitalWrite(REDLED, LOW);
    delay(delaymS);
  }
}


void setup()
{
  uint32_t available_PSRAM_size;
  uint32_t new_available_PSRAM_size;

  //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);      //disable brownout detector
  pinMode(REDLED, OUTPUT);                       //setup pin as output for indicator LED
  led_Flash(2, 125);                             //two quick LED flashes to indicate program start
  ARsetDTLED(REDLED);                            //setup LED pin for data transfer indicator

  digitalWrite(NSS, HIGH);
  pinMode(NSS, OUTPUT);                          //disable LoRa device for now

  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  //debug port, format is Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial2.println();
  Serial2.println(__FILE__);

  if (psramInit())
  {
    Serial2.println("PSRAM is correctly initialised");
    available_PSRAM_size = ESP.getFreePsram();
    Serial2.println((String)"PSRAM Size available: " + available_PSRAM_size);
  }
  else
  {
    Serial2.println("PSRAM not available");
    while (1);
  }

  Serial2.println("Allocate array in PSRAM");
  uint8_t *byte_array = (uint8_t *) ps_malloc(ARDTarraysize * sizeof(uint8_t));
  PSRAMptr = byte_array;                              //save the pointe to byte_array to global pointer

  new_available_PSRAM_size = ESP.getFreePsram();
  Serial2.println((String)"PSRAM Size available: " + new_available_PSRAM_size);
  Serial2.print("PSRAM array bytes allocated: ");
  Serial2.println(available_PSRAM_size - new_available_PSRAM_size);
  Serial2.println();
}
