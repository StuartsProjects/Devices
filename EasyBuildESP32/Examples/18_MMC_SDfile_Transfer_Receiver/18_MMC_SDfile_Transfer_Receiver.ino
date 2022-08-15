/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 14/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program uses data transfer (DT) packets to receive a file sent from another 
  board and saves it to the SD card of this NodeMCU ESP32. The sending microcontroller is either another
  NodemCU ESP32 using program 17_MMC_SDfile_Transfer_Transmitter.ino or an Arduino such as a DUE using
  program 233_SDfile_Transfer_Transmitter.ino from the SX12xx library. 

  DT packets can be used for transfering large amounts of data in a sequence of packets or segments,
  in a reliable and resiliant way. The remote file open request, the segements sent and the remote file close
  will be transmitted until a valid acknowledge comes from the receiver. 

  Each DT packet contains a variable length header array and a variable length data array as the payload.
  On transmission the NetworkID and CRC of the payload are appended to the end of the packet by the library
  routines. The use of a NetworkID and CRC ensures that the receiver can validate the packet to a high degree
  of certainty. The receiver will not accept packets that dont have the appropriate NetworkID or payload CRC
  at the end of the packet.

  The transmitter sends a sequence of segments in order and the receiver keeps track of the sequence. If
  the sequence fails for some reason, the receiver will return a NACK packet to the transmitter requesting
  the segment sequence it was expecting.

  Details of the packet identifiers, header and data lengths and formats used are in the file
  Data_transfer_packet_definitions.md in the \SX127X_examples\DataTransfer\ folder.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>

#include <SX127XLT.h>
#include <ProgramLT_Definitions.h>
#include "Settings.h"                        //LoRa settings etc.

SX127XLT LoRa;                               //create an SX127XLT library instance called LoRa, required by MMCtransfer.h

#define ENABLEMONITOR                        //enable monitor prints, turning off normal serial prints allows the Serial
                                             //port to be used for PC file transfers if needed.
#define PRINTSEGMENTNUM
#define ENABLEFILECRC                        //enable this define to use and show file CRCs
//#define DISABLEPAYLOADCRC                  //enable this define if you want to disable payload CRC checking
//#define DEBUG

#include "MMClibrary.h"                      //library of SD functions for MMC card mode
#include "MMCtransfer.h"                     //library of data transfer functions


void loop()
{
  SDreceiveaPacketDT();
}


void setup()
{
  pinMode(2, INPUT_PULLUP);                   //setup pin as output for indicator LED

#ifdef ENABLEMONITOR
  Monitorport.begin(115200);
  Monitorport.println();
  Monitorport.println(F(__FILE__));
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

 DTSD_printDirectory();

#ifdef ENABLEMONITOR
  Monitorport.println();
#endif

#ifdef DISABLEPAYLOADCRC
  LoRa.setReliableConfig(NoReliableCRC);
#endif

  if (LoRa.getReliableConfig(NoReliableCRC))
  {
    Monitorport.println(F("Payload CRC disabled"));
  }
  else
  {
#ifdef ENABLEMONITOR
    Monitorport.println(F("Payload CRC enabled"));
#endif
  }

  SDDTSegmentNext = 0;
  SDDTFileOpened = false;

#ifdef ENABLEMONITOR
  Monitorport.println(F("SDfile transfer receiver ready"));
  Monitorport.println();
#endif
}
