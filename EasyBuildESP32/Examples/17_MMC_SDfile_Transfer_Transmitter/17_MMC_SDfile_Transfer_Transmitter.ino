/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 14/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program uses data transfer (DT) packets to send a file from the SD card on this
  NodeMCU ESP32 to another microcontroller where it can be saved on the destination SD card. The other
  microcontroller can be another NodeMCU ESP32 running program 18_MMC_SDfile_Transfer_Receiver.ino or an
  Arduino such as a DUE running program 234_SDfile_Transfer_Receiver from the SX12XX library.

  DT packets can be used for transfering large amounts of data in a sequence of packets or segments,
  in a reliable and resiliant way. The file open requests to the remote receiver, each segement sent and
  the remote file close will all keep transmitting until a valid acknowledge comes from the receiver.
  
  On transmission the NetworkID and CRC of the payload are appended to the end of the packet by the library
  routines. The use of a NetworkID and CRC ensures that the receiver can validate the packet to a high degree
  of certainty.

  The transmitter sends the sequence of segments in order. If the sequence fails for some reason, the receiver
  will return a NACK packet to the transmitter requesting the segment sequence it was expecting.

  Details of the packet identifiers, header and data lengths and formats used are in the file;
  'Data transfer packet definitions.md' in the \SX127X_examples\DataTransfer\ folder.

  The transfer can be carried out using LoRa packets, max segment size (defined by DTSegmentSize) is 245 bytes
  for LoRa.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>

#include <SX127XLT.h>
#include <ProgramLT_Definitions.h>
#include "Settings.h"                        //LoRa settings etc.

SX127XLT LoRa;                               //create an SX127XLT library instance called LoRa, required by MMCtransfer.h

#define ENABLEMONITOR                        //enable monitor prints, turning off normal serial prints allows the Serial
                                             //port to be used for PC file transfers if needed.
#define PRINTSEGMENTNUM                      //enable this define to print segment numbers 
#define ENABLEFILECRC                        //enable this define to uses and show file CRCs
//#define DISABLEPAYLOADCRC                  //enable this define if you want to disable payload CRC checking

#include "MMClibrary.h"                      //library of SD functions for MMC card mode
#include "MMCtransfer.h"                     //library of data transfer functions

//choice of files to send
//char FileName[] = "/$50SATL.JPG";          //file length 63091 bytes, file CRC 0x59CE
char FileName[] = "/$50SATS.JPG";            //file length 6880 bytes, file CRC 0x0281
//char FileName[] = "/$50SATT.JPG";          //file length 1068 bytes, file CRC 0x6A02
//char FileName[] = "/testfile.txt";         //file length 512 bytes, file CRC 0x5FEB


void loop()
{
  uint32_t filelength;

#ifdef ENABLEMONITOR
  Monitorport.println(F("Transfer started"));
#endif

  filelength = SDsendFile(FileName, sizeof(FileName));

  if (filelength)
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("Transfer finished"));
#endif
  }
  else
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("Transfer failed"));
    Monitorport.println();
#endif
  }

  delay(15000);

}


void setup()
{
  pinMode(2, INPUT_PULLUP);                   //needed for MMC card to work

#ifdef ENABLEMONITOR
  Monitorport.begin(115200);
  Monitorport.println();
  Monitorport.println(__FILE__);
  Monitorport.println();
  Monitorport.println(__TIME__);
  Monitorport.println();
  Monitorport.println(__DATE__);
  Monitorport.println();
#endif

  SPI.begin(SCK, MISO, MOSI, NSS);

  if (LoRa.begin(NSS, NRESET, DIO0, LORA_DEVICE))
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("LoRa device found"));
#endif
  }
  else
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("LoRa device error - program halted"));
#endif
    while (1);
  }

  DTSD_printDirectory();

  LoRa.setupLoRa(Frequency, Offset, SpreadingFactor, Bandwidth, CodeRate, Optimisation);

#ifdef ENABLEMONITOR
  Monitorport.println();
  Monitorport.print(F("Initializing MMC card..."));
#endif

  if (DTSD_initSD())
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("MMC Card initialized."));
#endif
  }
  else
  {
    Monitorport.println(F("MMC Card failed - program halted"));
    while (1);
  }

#ifdef ENABLEMONITOR
  Monitorport.println();
#endif

#ifdef DISABLEPAYLOADCRC
  LoRa.setReliableConfig(NoReliableCRC);
#endif

  if (LoRa.getReliableConfig(NoReliableCRC))
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("Payload CRC disabled"));
#endif
  }
  else
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("Payload CRC enabled"));
#endif
  }

  SDDTFileTransferComplete = false;

#ifdef ENABLEMONITOR
  Monitorport.println(F("File transfer ready"));
  Monitorport.println();
#endif
}
