/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 10/06/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation -  This is a version of the SX12xx library data tranfer functions that can transmit
  and receive the contents of a memory array via LoRa. The receiver may receive the array directly into
  memory or store the received data segments to an SD card. These array transfer functions assume that
  ultimatly the array will be saved as a file somewhere, so a filename needs to be provided, even when
  it may not be used.

  These functions expect the calling sketch to create an SX12XX library instance called LoRa, so that
  SX12XX library functions are called like this; LoRa.getATTXNetworkID().

  The actual port for monitor prints is defined like this;

  #define Monitorport Serialx

 *******************************************************************************************************/

/*
  Issues:
*/

#define ENABLEMONITOR    //Enable monitor prints
#define PRINTSEGMENTNUM  //print segment numbers
//#define DEBUGTF
//#define PRINTHEADER
//#define PRINTACK
//#define ATPRINTRSSISNR
//#define PRINTSECONDS

//symbols for 127 byte packets, used to estimate packet air time of segment, starts at SF5
const PROGMEM uint16_t Symbols127Bytes[] = { 274, 229, 197, 177, 157, 142, 132, 122 };

//so that Monitorport prints default to the primary Monitorport port of Monitorport
#ifndef Monitorport
#define Monitorport Serial
#endif

#define ATUNUSED(v) (void)(v)  //add ATUNUSED(variable); to avoid compiler warnings

#include <arrayRW.h>  //part of SX12XX library

#define ATSegmentWrite 0xA0      //packet type for segment write
#define ATSegmentWriteACK 0xA1   //packet type for segment write ACK
#define ATSegmentWriteNACK 0xA2  //packet type for segment write NACK
#define ATSegmentWriteHeaderL 8

#define ATArrayStart 0xA4      //packet type for array start
#define ATArrayStartACK 0xA5   //packet type for array start ACK
#define ATArrayStartNACK 0xA6  //packet type for array start NACK
#define ATArrayStartHeaderL 14

#define ATArrayEnd 0xA8      //packet type for file end
#define ATArrayEndACK 0xA9   //packet type for file end
#define ATArrayEndNACK 0xAA  //packet type for file end
#define ATArrayEndHeaderL 12

#define ATStart 0xB0      //packet type for start (or restart) file transfer
#define ATStartACK 0xB1   //packet type for start (or restart) ACK
#define ATStartNACK 0xB2  //packet type for start (or restart) NACK
#define ATStartHeaderL 6  //header length for start (or restart) file transfer

#define ATSettings 0xC4      //packet type for ATSettings transmit, camera ID, Freq, SF, BW, CR
#define ATSettingsACK 0xC5   //packet type for ATSettings ACK
#define ATSettingsNACK 0xC6  //packet type for ATSettings NACK
#define ATSettings2 0xC7     //packet type for ATSettings2 transmit, ATSettings plus TXPow, StdImg, SentImg, Seg size, Pics, Sleep
#define ATSettings3 0xC8     //packet type for ATSettings2 transmit, ATSettings2 plus remainder of var structue
#define ATSettingsHeaderL 8  //header length for ATSettings

#define ATReady 0xD0      //packet type for camera ready packet
#define ATReadyACK1 0xD1  //packet type for receiver basic ACK, no settings
#define ATReadyACK2 0xD2  //packet type for receiver basic ACK, basic settings
#define ATReadyACK3 0xD3  //packet type for receiver basic ACK, extended settings
#define ATReadyNACK 0xD7  //packet type for receiver NACK to ready

#define ATWake 0xF8      //packet type for ATWake
#define ATWakeACK 0xF9   //packet type for ATWake ACK
#define ATWakeNACK 0xFA  //packet type for ATWake NACK
#define ATWakeHeaderL 8  //header length for ATWake

const uint8_t ATfilenamesize = 32;  //size of filename buffer

//Variables used by transfer functions on transmitter and receiver
uint8_t ATRXPacketL;                  //length of received packet
uint8_t ATRXPacketType;               //type of received packet, segment write, ACK, NACK etc
uint8_t ATRXHeaderL;                  //length of header
int16_t ATPacketRSSI;                 //stores RSSI of received packet
int8_t ATPacketSNR;                   //stores signal to noise ratio of received packet
int32_t ATSNRtotal;                   //keep a running total of the SNR values
uint16_t ATAckCount;                  //keep a count of acks that are received within timeout period
uint16_t ATNoAckCount;                //keep a count of acks not received within timeout period
uint16_t ATSourceArrayCRC;            //CRC returned of the remote received array
uint32_t ATSourceArrayLength;         //length of file at source\transmitter
uint16_t ATDestinationArrayCRC;       //CRC of complete array received
uint32_t ATDestinationArrayLength;    //length of file\array written on the destination\receiver
uint32_t ATMAXarraysize;              //maximum size or array that can be handled
uint32_t ATStartmS;                   //used for timeing transfers
uint16_t ATSegment = 0;               //current segment number
char ATFilenamebuff[ATfilenamesize];  //global buffer to store filename
uint8_t ATHeader[16];                 //header array
uint8_t ATData[245];                  //data/segment array
uint8_t ATTXFlags = 0;                //Transmitted flags byte used to pass status information from TX to RX nodes
uint16_t ATErrors;                    //can be used for tracking errors in the transfer process
bool ATArrayTransferComplete;         //bool to flag array transfer complete
uint32_t ATSendmS;                    //used for timing transfers
float ATSendSecs;                     //seconds to transfer a file
uint8_t ATSegmentSize;                //segment size in use
uint8_t ATType;                       //a number describing array image type
uint16_t ATNumberSegments;            //number of segments for a file transfer
uint8_t ATLastSegmentSize;            //size of the last segment
uint16_t ATLocalPayloadCRC;           //for calculating the local data array CRC
uint8_t ATTXPacketL;                  //length of transmitted packet
uint16_t ATTXNetworkID;               //this is used to store the 'network' number, receiver must have the same
uint16_t ATTXArrayCRC;                //should contain CRC of data array transmitted
uint16_t ATSentSegments;              //count of segments sent

//Receive mode only variables
uint16_t ATRXErrors;          //count of packets received with error during a transfer
uint32_t ATRXOK;              //count of packets received OK during a transfer
uint8_t ATRXFlags;            //Received flags byte used to pass status information from TX to RX nodes
uint8_t ATRXDataarrayL;       //length of data array\segment
bool ATArrayStarted;          //bool to flag when array write has started
bool ATArrayEnded;            //bool to flag when array write has finished
bool ATArrayTimeout;          //set true when there is a timeout waiting for transfer
uint16_t ATSegmentNext;       //next segment expected
uint16_t ATReceivedSegments;  //count of segments received
uint16_t ATSegmentLast;       //last segment processed

//A global pointer to the array and a variable for its length and current location are used so that all routines
//have access to the array to send without constantly passing the array pointer and variables between functions.
uint8_t *ATptrsendArray;     //create a global pointer to the array to send, so all functions have access
uint8_t *ATptrreceivearray;  //create a global pointer to the array to receive into, so all functions have access
uint32_t ATArrayLength;      //length of array to send or receive
uint32_t ATArraylocation;    //a global variable giving the location in the array last used

//Transmitter mode functions
bool ATsendArray(uint8_t *ptrarray, uint32_t arraylength, char *filename, uint8_t namelength, uint8_t segmentsize, uint8_t arraytype, uint8_t flags);
bool ATStartArrayTransfer(char *buff, uint8_t filenamesize);
bool ATsendSegments();
bool ATsendArraySegment(uint16_t segnum, uint8_t segmentsize);
void ATprintRSSISNR();
bool ATendArrayTransfer(char *buff, uint8_t filenamesize);
void ATbuild_ATArrayStartHeader(uint8_t *header, uint8_t headersize, uint8_t datalength, uint32_t arraylength, uint16_t arraycrc, uint8_t segsize, uint8_t arraytype);
void ATbuild_DTSegmentHeader(uint8_t *header, uint8_t headersize, uint8_t datalen, uint16_t segnum);
void ATbuild_ATArrayEndHeader(uint8_t *header, uint8_t headersize, uint8_t datalength, uint32_t arraylength, uint16_t arraycrc, uint8_t segsize);
void ATprintLocalArrayDetails();
void ATprintSeconds();
void ATprintAckReception();
void ATprintACKdetail();
void ATprintdata(uint8_t *dataarray, uint8_t arraysize);
uint16_t ATgetNumberSegments(uint32_t arraysize, uint8_t segmentsize);
uint8_t ATgetLastSegmentSize(uint32_t arraysize, uint8_t segmentsize);
bool ATsendATInfo();
void ATbuild_ATInfoHeader(uint8_t *header, uint8_t headersize, uint8_t datalen);

//Receiver mode functions
uint32_t ATreceiveArray(uint8_t *arraychar, uint32_t length, uint32_t receivetimeout);
bool ATreceivePacketDT();
void ATreadHeaderDT();
bool ATprocessPacket(uint8_t packettype);
bool ATprocessATSettings();
bool ATprocessSegmentWrite();
bool ATprocessArrayStart(uint8_t *buff, uint8_t filenamesize);
bool ATprocessArrayEnd();
uint16_t ATreturn_RXOK();
uint16_t ATreturn_ATRXErrors();
int32_t ATreturn_SNRtotal();
uint8_t ATreturn_SegmentSize();
uint8_t ATreturn_ATtype();
void ATprintSourceArrayDetails();
void ATprintDestinationArrayDetails();
uint16_t ATarrayCRC(uint8_t *buffer, uint32_t size, uint16_t startvalue);

//Common functions
void ATprintheader(uint8_t *hdr, uint8_t hdrsize);
void ATprintArrayHEX(uint8_t *buff, uint32_t len);
void ATprintReliableStatus();
void ATprintPacketDetails();
uint16_t ATestimatePacketTime(uint8_t segsize, uint8_t sf, uint8_t bw);

//bit numbers used by ATDTErrors (16bits) and TX\RX Errors (first 8bits)
const uint8_t TXLoadFileError = 0;  //set bit number if TX has an error loading image file


//************************************************
//Transmit mode functions
//************************************************

bool ATsendArray(uint8_t *ptrarray, uint32_t arraylength, char *filename, uint8_t namelength, uint8_t segmentsize, uint8_t arraytype, uint8_t flags) {

  uint8_t localattempts = 0;
  memcpy(ATFilenamebuff, filename, namelength);  //copy the name of destination file into global filename array for use outside this function
  ATptrsendArray = ptrarray;                     //set global pointer to array pointer passed
  ATArrayLength = arraylength;                   // the length of array to send
  ATSourceArrayCRC = 0;
  ATSourceArrayLength = 0;
  ATDestinationArrayCRC = 0;
  ATDestinationArrayLength = 0;
  ATSegmentSize = segmentsize;  //segment size being used for transfer
  ATType = arraytype;           //a number describing array type
  ATRXFlags = flags;            //Flags byte received from transmitter

  Monitorport.println(F("ATsendArray()"));
  Monitorport.print(F("ATTXFlags "));
  Monitorport.println(ATTXFlags);

  do {
    localattempts++;
    ATNoAckCount = 0;
    ATStartmS = millis();

#ifdef ENABLEMONITOR
    Monitorport.print(F("Send array attempt "));
    Monitorport.println(localattempts);
#endif

    if (ATStartArrayTransfer(filename, namelength)) {
#ifdef ENABLEMONITOR
      Monitorport.println(F("Array write started OK on remote"));
      ATprintLocalArrayDetails();
#endif
    } else {
#ifdef ENABLEMONITOR
      Monitorport.println(F("ERROR creating remote array"));
#endif

      ATArrayTransferComplete = false;
      continue;
    }

    delay(var.ATFunctionDelaymS);

    if (!ATsendSegments()) {
#ifdef ENABLEMONITOR
      Monitorport.println();
      Monitorport.println(F("ERROR in sendSegments()"));
      Monitorport.println();
#endif
      ATArrayTransferComplete = false;
      continue;
    }

    delay(var.ATFunctionDelaymS);

    if (ATendArrayTransfer(filename, namelength))  //send command to end remote array write
    {
      ATSendmS = millis() - ATStartmS;  //record time taken for transfer
      beginarrayRW(ATHeader, 4);
      ATDestinationArrayLength = arrayReadUint32();

#ifdef ENABLEMONITOR
      Monitorport.println(F("Array write OK on remote"));
      Monitorport.print(F("Acknowledged remote destination array length "));
      Monitorport.println(ATDestinationArrayLength);
#endif

      if (ATDestinationArrayLength != ATSourceArrayLength) {
#ifdef ENABLEMONITOR
        Monitorport.println(F("*******"));
        Monitorport.println(F("ERROR destination array and local array lengths do not match"));
        Monitorport.println(F("Restarting transfer"));
        Monitorport.println(F("*******"));
#endif

        ATArrayTransferComplete = false;
        continue;
      } else {

#ifdef ENABLEMONITOR
        Monitorport.println(F("Destination array and local array lengths match"));
#endif
      }
      ATDestinationArrayCRC = arrayReadUint16();

#ifdef ENABLEMONITOR
      Monitorport.print(F("Acknowledged destination array CRC 0x"));
      Monitorport.println(ATDestinationArrayCRC, HEX);
#endif

      if (ATDestinationArrayCRC != ATSourceArrayCRC) {
#ifdef ENABLEMONITOR
        Monitorport.println(F("ERROR destination array and local array CRCs do not match"));
        Monitorport.println(F("Restarting transfer"));
#endif

        ATArrayTransferComplete = false;
        continue;
      } else {
#ifdef ENABLEMONITOR
        Monitorport.println(F("Destination array and local array CRCs match"));
#endif
      }
      ATArrayTransferComplete = true;
    } else {
#ifdef ENABLEMONITOR
      Monitorport.println(F("ERROR ending destination array write"));
      Monitorport.println(F("Restarting transfer"));
#endif

      ATArrayTransferComplete = false;
      continue;
    }
  } while ((!ATArrayTransferComplete) && (localattempts < var.ATStartAttempts));

  if (localattempts == var.ATStartAttempts) {
    return false;
  }

  ATSendSecs = (float)ATSendmS / 1000;

#ifdef ENABLEMONITOR
  Monitorport.print(F("NoAckCount "));
  Monitorport.println(ATNoAckCount);
  Monitorport.print(F("Transmit time "));
  Monitorport.print(ATSendSecs, 2);
  Monitorport.println(F("secs"));
  Monitorport.print(F("Transmit rate "));
  Monitorport.print((ATDestinationArrayLength * 8) / (ATSendSecs), 0);
  Monitorport.println(F("bps"));
#endif

  return true;
}


bool ATStartArrayTransfer(char *buff, uint8_t filenamesize) {
  //Start transfer of array to remote array or file
  uint8_t ACKlength;
  uint8_t localattempts = 0;

  ATRXErrors = 0;

#ifdef ENABLEMONITOR
  Monitorport.print(F("Start array transfer for "));
  Monitorport.println(buff);
#endif

  ATSourceArrayLength = ATArrayLength;

  if (ATType == 255) {
    Monitorport.println(F("Transmitter file error"));
  } else {
    if (ATSourceArrayLength == 0) {
#ifdef ENABLEMONITOR
      Monitorport.print(F("Error - array 0 bytes nothing to transfer "));
      Monitorport.println(buff);
#endif
      return false;
    }
  }

  ATSourceArrayCRC = ATarrayCRC((uint8_t *)ATptrsendArray, ATArrayLength, 0xFFFF);  //get array CRC from position 0 to end
  ATNumberSegments = ATgetNumberSegments(ATSourceArrayLength, ATSegmentSize);
  ATLastSegmentSize = ATgetLastSegmentSize(ATSourceArrayLength, ATSegmentSize);
  ATbuild_ATArrayStartHeader(ATHeader, ATArrayStartHeaderL, filenamesize, ATSourceArrayLength, ATSourceArrayCRC, ATSegmentSize, ATType);
  ATLocalPayloadCRC = ATarrayCRC((uint8_t *)buff, filenamesize, 0xFFFF);

  do {
    localattempts++;

#ifdef ENABLEMONITOR
    Monitorport.print(F("Send open remote file request attempt "));
    Monitorport.println(localattempts);
#endif
    uint32_t startmS = millis();
    ATTXPacketL = LoRa.transmitDT(ATHeader, ATArrayStartHeaderL, (uint8_t *)buff, filenamesize, var.NetworkID, var.ATTXtimeoutmS, var.TXpower, WAIT_TX);
    ATTXNetworkID = LoRa.getTXNetworkID(ATTXPacketL);  //get the networkID appended to packet
    ATTXArrayCRC = LoRa.getTXPayloadCRC(ATTXPacketL);  //get the payload CRC thats was appended to packet

#ifdef ENABLEMONITOR
    if (ATTXPacketL == 0)  //if there has been a send and ack error, ATTXPacketL returns as 0
    {
      Monitorport.println(F("Transmit error"));
    }
#endif

    ACKlength = LoRa.waitACKDT(ATHeader, ATArrayStartHeaderL, var.ATACKopentimeoutmS);
    ATRXPacketType = ATHeader[0];

    if ((ACKlength > 0) && (ATRXPacketType == ATArrayStartACK)) {
#ifdef PRINTACK
      Monitorport.print(F("ACK > "));
      if (ACKlength > 16) {
        ACKlength = 16;
      }
      ATprintArrayHEX(ATHeader, ACKlength);
      Monitorport.println();
#endif

      if (ATType == FILEERROR) {
        Monitorport.println(F("Transmitter file error for requested image"));
        return false;
      }

    } else {
      ATNoAckCount++;

#ifdef DEBUGTF
      Monitorport.println(F("NoACK"));
#endif

      if (ATNoAckCount > var.ATNoAckCountLimit) {
#ifdef ENABLEMONITOR
        Monitorport.println(F("ERROR NoACK limit reached"));
        Monitorport.println();
#endif
        return false;
      }
    }
  } while ((ACKlength == 0) && (localattempts < var.ATSendAttempts));

  if (localattempts == var.ATSendAttempts) {
    return false;
  }
  return true;
}


bool ATsendSegments() {
  //Start the array transfer at segment 0
  ATSegment = 0;
  ATSentSegments = 0;

  ATArraylocation = 0;  //start at first position in array

  while (ATSegment < (ATNumberSegments - 1)) {
#ifdef PRINTSECONDS
    ATprintSeconds();
#endif

    if (ATsendArraySegment(ATSegment, ATSegmentSize)) {
      ATSentSegments++;
    } else {
      return false;
    }
    delay(var.ATFunctionDelaymS);
  };

#ifdef ENABLEMONITOR
  Monitorport.print(F("Last segment "));
#endif

  if (!ATsendArraySegment(ATSegment, ATLastSegmentSize)) {
    return false;
  }
  return true;
}


bool ATsendArraySegment(uint16_t segnum, uint8_t segmentsize) {
  //Send array segment as payload in a data transfer packet
  uint8_t ACKlength;
  uint8_t index;
  uint8_t tempdata;
  uint8_t localattempts = 0;

  for (index = 0; index < segmentsize; index++) {
    tempdata = ATptrsendArray[ATArraylocation];
    ATData[index] = tempdata;
    ATArraylocation++;
  }

  ATbuild_DTSegmentHeader(ATHeader, ATSegmentWriteHeaderL, segmentsize, segnum);

#ifdef PRINTSEGMENTNUM
  Monitorport.print(segnum);
#ifndef ATprintRSSISNR
  Monitorport.println();
#endif
#endif


#ifdef PRINTHEADER
  Monitorport.print(F(" "));
  ATprintheader(ATHeader, ATSegmentWriteHeaderL);
  Monitorport.print(F(" "));
  ATprintdata(ATData, segmentsize);  //print segment size of data array only
  Monitorport.println();
#endif


  do {
    localattempts++;

    ATTXPacketL = LoRa.transmitDT(ATHeader, ATSegmentWriteHeaderL, (uint8_t *)ATData, segmentsize, var.NetworkID, var.ATTXtimeoutmS, var.TXpower, WAIT_TX);

    if (ATTXPacketL == 0)  //if there has been an error ATTXPacketL returns as 0
    {
#ifdef ENABLEMONITOR
      Monitorport.println(F("Transmit error"));
#endif
    }

    ACKlength = LoRa.waitACKDT(ATHeader, ATSegmentWriteHeaderL, var.ATACKsegtimeoutmS);
    ATRXPacketType = ATHeader[0];

    if (ACKlength > 0) {

#ifdef ATprintRSSISNR
      ATPacketSNR = LoRa.readPacketSNR();
      ATPacketRSSI = LoRa.readPacketRSSI();
      ATprintRSSISNR();
      Monitorport.println();
#endif

      if (ATRXPacketType == ATSegmentWriteNACK) {
        ATSegment = ATHeader[4] + (ATHeader[5] << 8);  //load what the segment number should be
        ATRXHeaderL = ATHeader[2];
        ATArraylocation = ATSegment * ATSegmentSize;

#ifdef ENABLEMONITOR
        Monitorport.println();
        Monitorport.print(F("Received restart request at segment "));
        Monitorport.println(ATSegment);
#ifdef PRINTHEADER
        ATprintheader(ATHeader, ATRXHeaderL);
#endif
        Monitorport.println();
        Monitorport.print(F("Seek to array location "));
        Monitorport.println(ATSegment * ATSegmentSize);
        Monitorport.println();
#endif
      }

      if (ATRXPacketType == ATStartNACK) {
#ifdef ENABLEMONITOR
        Monitorport.println(F("Received restart request"));
#endif

        return false;
      }

      //ack is valid, segment was acknowledged if here

      if (ATRXPacketType == ATSegmentWriteACK) {
        ATAckCount++;
        ATSegment++;  //increase value for next segment
        return true;
      }
    } else {
      ATNoAckCount++;

#ifdef DEBUGTF
      Monitorport.println(F("NoACK"));
#endif

      if (ATNoAckCount > var.ATNoAckCountLimit) {
#ifdef ENABLEMONITOR
        Monitorport.println(F("ERROR NoACK limit reached"));
#endif

        return false;
      }
    }
  } while ((ACKlength == 0) && (localattempts < var.ATSendAttempts));

  if (localattempts == var.ATSendAttempts) {
    return false;
  }

  return true;
}


void ATprintRSSISNR() {
  Monitorport.print(F(" "));
  Monitorport.print(ATPacketRSSI);
  Monitorport.print(F("dBm"));
  Monitorport.print(F(","));
  Monitorport.print(ATPacketSNR);
  Monitorport.print(F("dB "));
}


bool ATendArrayTransfer(char *buff, uint8_t filenamesize) {
  //End array transfer
  uint8_t ValidACK;
  uint8_t localattempts = 0;

  ATbuild_ATArrayEndHeader(ATHeader, ATArrayEndHeaderL, filenamesize, ATSourceArrayLength, ATSourceArrayCRC, ATSegmentSize);

  do {
    localattempts++;

    ATTXPacketL = LoRa.transmitDT(ATHeader, ATArrayEndHeaderL, (uint8_t *)buff, filenamesize, var.NetworkID, var.ATTXtimeoutmS, var.TXpower, WAIT_TX);
    ATTXNetworkID = LoRa.getTXNetworkID(ATTXPacketL);
    ATTXArrayCRC = LoRa.getTXPayloadCRC(ATTXPacketL);

    if (ATTXPacketL == 0)  //if there has been a send and ack error, ATATTXPacketL returns as 0
    {
#ifdef ENABLEMONITOR
      Monitorport.println(F("Transmit error"));
#endif
    }

    ValidACK = LoRa.waitACKDT(ATHeader, ATArrayEndHeaderL, var.ATACKclosetimeoutmS);
    ATRXPacketType = ATHeader[0];

    if ((ValidACK > 0) && (ATRXPacketType == ATArrayEndACK)) {
#ifdef PRINTHEADER
      Monitorport.print(F("ACK header "));
      ATprintArrayHEX(ATHeader, ValidACK);  //ValidACK is packet length
      Monitorport.println();
#endif
    } else {
      ATNoAckCount++;

#ifdef DEBUGTF
      Monitorport.println(F("NoACK"));
#endif

      if (ATNoAckCount > var.ATNoAckCountLimit) {
#ifdef ENABLEMONITOR
        Monitorport.println(F("ERROR NoACK limit reached"));
#endif
        return false;
      }

#ifdef PRINTATRAYHEX
      ATprintArrayHEX(ATHeader, ValidACK);  //ValidACK is packet length
#endif
      Monitorport.println();
    }
  } while ((ValidACK == 0) && (localattempts < var.ATSendAttempts));


  if (localattempts == var.ATSendAttempts) {
    return false;
  }
  return true;
}


void ATbuild_ATArrayStartHeader(uint8_t *header, uint8_t headersize, uint8_t datalength, uint32_t arraylength, uint16_t arraycrc, uint8_t segsize, uint8_t arraytype) {
  //This builds the header buffer for the filename receiver might save the array as

  beginarrayRW(header, 0);        //start writing to array at location 0
  arrayWriteUint8(ATArrayStart);  //byte 0, write the packet type
  arrayWriteUint8(ATRXFlags);     //byte 1, ATRXFlags byte
  arrayWriteUint8(headersize);    //byte 2, write length of header
  arrayWriteUint8(datalength);    //byte 3, write length of dataarray
  arrayWriteUint32(arraylength);  //byte 4,5,6,7, write the array length
  arrayWriteUint16(arraycrc);     //byte 8, 9, write array CRC
  arrayWriteUint8(segsize);       //byte 10, segment size
  arrayWriteUint8(arraytype);     //byte 11, type of array
  endarrayRW();
}


void ATbuild_DTSegmentHeader(uint8_t *header, uint8_t headersize, uint8_t datalen, uint16_t segnum) {
  //This builds the header buffer for a segment transmit

  beginarrayRW(header, 0);          //start writing to array at location 0
  arrayWriteUint8(ATSegmentWrite);  //write the packet type
  arrayWriteUint8(ATTXFlags);       //initial ATFlags byte
  arrayWriteUint8(headersize);      //write length of header
  arrayWriteUint8(datalen);         //write length of data array
  arrayWriteUint16(segnum);         //write the DTsegment number
  endarrayRW();
}


void ATbuild_ATArrayEndHeader(uint8_t *header, uint8_t headersize, uint8_t datalength, uint32_t arraylength, uint16_t arraycrc, uint8_t segsize) {
  //This builds the header buffer for the end remote array write

  beginarrayRW(header, 0);        //start writing to array at location 0
  arrayWriteUint8(ATArrayEnd);    //byte 0, write the packet type
  arrayWriteUint8(ATTXFlags);     //byte 1, initial ATFlags byte
  arrayWriteUint8(headersize);    //byte 2, write length of header
  arrayWriteUint8(datalength);    //byte 3, write length of dataarray
  arrayWriteUint32(arraylength);  //byte 4,5,6,7, write the array length
  arrayWriteUint16(arraycrc);     //byte 8, 9, write array CRC
  arrayWriteUint8(segsize);       //byte 10, segment size
  arrayWriteUint8(0);             //byte 11, unused byte
  endarrayRW();
}


void ATprintLocalArrayDetails() {
#ifdef ENABLEMONITOR
  Monitorport.print(F("Source array length "));
  Monitorport.print(ATSourceArrayLength);
  Monitorport.println(F(" bytes"));
  Monitorport.print(F("Source array CRC is 0x"));
  Monitorport.println(ATSourceArrayCRC, HEX);
  Monitorport.print(F("Segment Size "));
  Monitorport.println(ATSegmentSize);
  Monitorport.print(F("Number segments "));
  Monitorport.println(ATNumberSegments);
  Monitorport.print(F("Last segment size "));
  Monitorport.println(ATLastSegmentSize);
#endif
}


void ATprintSeconds() {
#ifdef ENABLEMONITOR
  float secs;
  secs = ((float)millis() / 1000);
  Monitorport.print(secs, 2);
  Monitorport.print(F(" "));
#endif
}


void ATprintAckReception() {
#ifdef ENABLEMONITOR
  ATPacketRSSI = LoRa.readPacketRSSI();
  ATPacketSNR = LoRa.readPacketSNR();
  Monitorport.print(F("ATAckCount,"));
  Monitorport.print(ATAckCount);
  Monitorport.print(F(",ATNoAckCount,"));
  Monitorport.print(ATNoAckCount);
  Monitorport.print(F(",AckRSSI,"));
  Monitorport.print(ATPacketRSSI);
  Monitorport.print(F("dBm,AckSNR,"));
  Monitorport.print(ATPacketSNR);
  Monitorport.print(F("dB"));
  Monitorport.println();
#endif
}


void ATprintACKdetail() {
#ifdef ENABLEMONITOR
  Monitorport.print(F("ACKDetail"));
  Monitorport.print(F(",RXNetworkID,0x"));
  Monitorport.print(LoRa.getRXNetworkID(ATRXPacketL), HEX);
  Monitorport.print(F(",RXPayloadCRC,0x"));
  Monitorport.print(LoRa.getRXPayloadCRC(ATRXPacketL), HEX);
  Monitorport.print(F(",ATRXPacketL,"));
  Monitorport.print(ATRXPacketL);
  Monitorport.print(F(" "));
  ATprintReliableStatus();
  Monitorport.println();
#endif
}


void ATprintdata(uint8_t *dataarray, uint8_t arraysize) {
  ATUNUSED(dataarray);
  ATUNUSED(arraysize);

#ifdef ENABLEMONITOR
  Monitorport.print(F("DataBytes,"));
  Monitorport.print(arraysize);
  Monitorport.print(F("  "));
  ATprintArrayHEX((uint8_t *)dataarray, 16);  //There is a lot of data to print so only print first 16 bytes
#endif
}


uint16_t ATgetNumberSegments(uint32_t arraysize, uint8_t segmentsize) {
  uint16_t segments;
  segments = arraysize / segmentsize;

  if ((arraysize % segmentsize) > 0) {
    segments++;
  }
  return segments;
}


uint8_t ATgetLastSegmentSize(uint32_t arraysize, uint8_t segmentsize) {
  uint8_t lastsize;

  lastsize = arraysize % segmentsize;
  if (lastsize == 0) {
    lastsize = segmentsize;
  }
  return lastsize;
}


//************************************************
//Receiver mode  functions
//************************************************

uint32_t ATreceiveArray(uint8_t *ptrarray, uint32_t length, uint32_t receivetimeout) {
  //returns 0 if no ATArrayEnded set, returns length of array if received
  uint32_t startmS;

  ATptrreceivearray = ptrarray;  //set global pointer to array pointer passed
  ATMAXarraysize = length;
  ATArrayTimeout = false;
  ATArrayEnded = false;
  ATDestinationArrayLength = 0;
  ATReceivedSegments = 0;
  ATRXOK = 0;

  startmS = millis();

  do {
    if (ATreceivePacketDT()) {
      startmS = millis();
      if (ATArrayTimeout) {
        break;
      }
    }

    if (ATArrayEnded)  //has the end array transfer been received ?
    {
      return ATDestinationArrayLength;
    }
  } while (((uint32_t)(millis() - startmS) < receivetimeout));


  if (ATArrayEnded)  //has the end array transfer been received ?
  {
    return ATDestinationArrayLength;
  } else {
    ATArrayTimeout = true;
    Monitorport.println(F("ATArrayTimeout"));
    return 0;
  }

  return 0;
}


bool ATreceivePacketDT() {
  //Receive data transfer packets
  uint8_t readchar;

  if (Monitorport.available() > 0) {
    Monitorport.println(F("Serial character during transfer - terminate"));
    readchar = Monitorport.read();
    ATArrayTimeout = true;
    return 0;
  }

  ATRXPacketType = 0;
  ATRXPacketL = LoRa.receiveDT(ATHeader, var.ATHeaderSizeMax, (uint8_t *)ATData, var.ATDataSizeMax, var.NetworkID, var.ATRXtimeoutmS, WAIT_RX);

#ifdef PRINTSECONDS
  ATprintSeconds();
#endif

  if (ATRXPacketL > 0) {
    //if the LoRa.receiveDT() returns a value > 0 for ATRXPacketL then packet was received OK
    //then only action payload if destinationNode = thisNode

    ATPacketSNR = LoRa.readPacketSNR();
    ATSNRtotal = ATSNRtotal + ATPacketSNR;  //increment SNR total, set to zero at array transfer start

#ifdef ATprintRSSISNR
    ATPacketRSSI = LoRa.readPacketRSSI();
    ATprintRSSISNR();
#endif

    ATreadHeaderDT();                 //get the basic header details into global variables ATRXPacketType etc
    ATprocessPacket(ATRXPacketType);  //process and act on the packet

    return true;
  } else {
    //if here the LoRa.receiveDT() function detected an error RXOK is 0

    uint16_t IRQStatus = LoRa.readIrqStatus();

    if (IRQStatus & IRQ_RX_TIMEOUT) {
      Monitorport.println(F("RX Timeout"));
    } else {
      ATRXErrors++;  //keep track of packet errors during transfer

#ifdef ENABLEMONITOR
      Monitorport.print(F("PacketError"));
      ATprintPacketDetails();
      Monitorport.println();
#endif
    }

    return false;
  }
}


void ATreadHeaderDT() {
  //The first 6 bytes of the header contain the important stuff, so load it up
  //so we can decide what to do next.
  beginarrayRW(ATHeader, 0);          //start buffer read at location 0
  ATRXPacketType = arrayReadUint8();  //load the packet type
  ATRXFlags = arrayReadUint8();       //ATFlags byte sent in ATArrayStart, ATSegmentWrite, ATArrayEnd headers
  ATRXHeaderL = arrayReadUint8();     //load the header length
  ATRXDataarrayL = arrayReadUint8();  //load the datalength
  ATSegment = arrayReadUint16();      //load the segment number
}


bool ATprocessPacket(uint8_t packettype) {
  //Decide what to do with an incoming packet

  if (packettype == ATSegmentWrite) {
    ATprocessSegmentWrite();
    return true;
  }

  if (packettype == ATArrayStart) {
    ATprocessArrayStart(ATData, ATRXDataarrayL);  //ATData contains the filename
    return true;
  }

  if (packettype == ATArrayEnd) {
    ATprocessArrayEnd();
    return true;
  }

  if (packettype == ATSettings) {
    ATprocessATSettings();
    return true;
  }

  return true;
}


bool ATprocessATSettings() {
  Monitorport.println(F("ATSettings Request Received"));
  return true;
}


bool ATprocessSegmentWrite() {
  //There is a request to write a segment to array on receiver
  //checks that the sequence of segment writes is correct

  uint8_t index, byteswritten = 0;

  ATRXOK++;

  if (!ATArrayStarted) {
    //something is wrong, have received a request to write a segment but there is no array
    //write in progresss so need to reject the segment write with a restart NACK

#ifdef ENABLEMONITOR
    Monitorport.println();
    Monitorport.println(F("Error - Segment write with no array write started - send NACK"));
    Monitorport.println();
#endif

    ATHeader[0] = ATStartNACK;
    delay(var.ATACKdelaymS);
    delay(var.ATDuplicatedelaymS);

    LoRa.sendACKDT(ATHeader, ATStartHeaderL, var.TXpower);

    return false;
  }

  if (ATSegment == ATSegmentNext) {
    //segment to write is as expected

    for (index = 0; index < ATRXDataarrayL; index++) {
      ATptrreceivearray[ATArraylocation] = ATData[index];
      ATArraylocation++;
      byteswritten++;
    }

#ifdef ENABLEMONITOR
#ifdef PRINTSEGMENTNUM
    Monitorport.print(ATSegment);
    Monitorport.println();
#endif
#endif

    ATHeader[0] = ATSegmentWriteACK;
    delay(var.ATACKdelaymS);

    LoRa.sendACKDT(ATHeader, ATSegmentWriteHeaderL, var.TXpower);

    ATReceivedSegments++;
    ATSegmentLast = ATSegment;  //so we can tell if sequece has been received twice
    ATSegmentNext = ATSegment + 1;
    return true;
  }

  if (ATSegment == ATSegmentLast) {

#ifdef DEBUGTF
    Monitorport.print(F("ERROR segment "));
    Monitorport.print(ATSegment);
    Monitorport.println(F(" already received "));
#endif

    ATHeader[0] = ATSegmentWriteACK;
    delay(var.ATDuplicatedelaymS);
    delay(var.ATACKdelaymS);

    LoRa.sendACKDT(ATHeader, ATSegmentWriteHeaderL, var.TXpower);

    return true;
  }

  if (ATSegment != ATSegmentNext) {
    ATHeader[0] = ATSegmentWriteNACK;
    ATHeader[4] = lowByte(ATSegmentNext);
    ATHeader[5] = highByte(ATSegmentNext);
    delay(var.ATACKdelaymS);
    delay(var.ATDuplicatedelaymS);  //add an extra delay here to stop repeated segment sends

#ifdef ENABLEMONITOR
    Monitorport.print(F(" ERROR Received Segment "));
    Monitorport.print(ATSegment);
    Monitorport.print(F(" expected "));
    Monitorport.print(ATSegmentNext);
    Monitorport.print(F(" "));
    Monitorport.print(F(" Send NACK for segment "));
    Monitorport.print(ATSegmentNext);
    Monitorport.println();
    Monitorport.println();
    Monitorport.print(F("Transmit restart request for segment "));
    Monitorport.println(ATSegmentNext);
#ifdef PRINTHEADER
    ATprintheader(ATHeader, ATRXHeaderL);
#endif
    Monitorport.println();
    Monitorport.println();
#endif

    LoRa.sendACKDT(ATHeader, ATSegmentWriteHeaderL, var.TXpower);
    return false;
  }

  return true;
}


bool ATprocessArrayStart(uint8_t *buff, uint8_t filenamesize) {
  //There is a request to start writing to a local array on receiver
  //The transmitter will have passed a filename to be used when saving the array to SD
  //or for use when transferring the array elsewhere

  Monitorport.println(F("Starting array transfer"));

  ATSNRtotal = 0;
  ATRXOK;
  ATRXErrors = 0;          //keep track of RX errors during array transfer
  ATArrayStarted = false;  //until this function completes the Array transfer has not started
  ATArrayEnded = false;    //and it cannot hve completed either ......
  ATArraylocation = 0;
  beginarrayRW(ATHeader, 4);                //start buffer read at location 4
  ATSourceArrayLength = arrayReadUint32();  //load the array length being sent
  ATSourceArrayCRC = arrayReadUint16();     //load the CRC of the array being sent
  ATSegmentSize = arrayReadUint8();         //read segment size in use from header
  ATType = arrayReadUint8();                //read array type from header

  Monitorport.print(F("Image type "));
  Monitorport.println(ATType);

  if (ATSourceArrayLength > ATMAXarraysize) {
    Monitorport.print(F("Array length to transfer "));
    Monitorport.print(ATSourceArrayLength);
    Monitorport.println(F(" bytes"));
    Monitorport.println(F("ERROR - Not enough memory allocated"));
    Monitorport.print(F("ATMAXarraysize is defined as "));
    Monitorport.print(ATMAXarraysize);
    Monitorport.println(F(" bytes"));
    Monitorport.println();
    return false;
  }

  memset(ATFilenamebuff, 0, ATfilenamesize);   //clear ATFilenamebuff to all 0s
  memcpy(ATFilenamebuff, buff, filenamesize);  //copy received ATData into ATFilenamebuff, the array should have a destination filename

#ifdef PRINTHEADER
  Monitorport.print(F("Header > "));
  ATprintArrayHEX(ATHeader, var.ATHeaderSizeMax);
#endif

#ifdef ENABLEMONITOR
  ATprintSourceArrayDetails();
#endif

  ATStartmS = millis();
  delay(var.ATACKdelaystartendmS);  //there needs to be a delay here, to wait for receiver to be ready

  ATHeader[0] = ATArrayStartACK;  //set the ACK packet type

  LoRa.sendACKDT(ATHeader, ATArrayStartHeaderL, var.TXpower);
  ATSegmentNext = 0;  //after array write start open, segment 0 is next

  if (ATRXFlags) {
    if (bitRead(ATRXFlags, TXLoadFileError)) {
      Monitorport.println();
      Monitorport.print(F("Transmitter load file error for image "));
      Monitorport.println(var.RequestedImage);
      Monitorport.println();
      var.RequestedImage = 0;
      bitClear(ATRXFlags, TXLoadFileError);
      ATArrayEnded = true;  //stop further attempts at array transfer
    }
  } else {
    Monitorport.println(F("No ATRXFlags set"));
  }
  ATArrayStarted = true;
  return true;
}


bool ATprocessArrayEnd() {
  // There is a request to end writing to an array on receiver

  uint8_t errorpercentage;

#ifdef ENABLEMONITOR
  Monitorport.println();
  Monitorport.print((char *)ATFilenamebuff);
  Monitorport.println(F(" end array write request"));
#endif

  ATRXOK++;

  if (ATArrayStarted)  //check if array write had been started, end it if it is
  {
    ATArrayStarted = false;
    ATDestinationArrayLength = ATArraylocation;
    ATDestinationArrayCRC = ATarrayCRC(ATptrreceivearray, ATDestinationArrayLength, 0xFFFF);

    beginarrayRW(ATHeader, 4);                   //start writing to array at location 12
    arrayWriteUint32(ATDestinationArrayLength);  //write array length of array just written just written to ACK header
    arrayWriteUint16(ATDestinationArrayCRC);     //write CRC of array just written to ACK header

#ifdef ENABLEMONITOR
    Monitorport.print(F("Time "));
    Monitorport.print(millis() - ATStartmS);
    Monitorport.println(F("mS"));
    ATprintDestinationArrayDetails();
#endif

  } else {
#ifdef ENABLEMONITOR
    Monitorport.println(F("Array write already ended"));
#endif
    delay(var.ATDuplicatedelaymS);
  }

  delay(var.ATACKdelaystartendmS);
  ATHeader[0] = ATArrayEndACK;

  LoRa.sendACKDT(ATHeader, ATArrayEndHeaderL, var.TXpower);

  ATArrayEnded = true;
  return true;
}


uint16_t ATreturn_RXOK() {
  return ATRXOK;
}


uint16_t ATreturn_ATRXErrors() {
  return ATRXErrors;
}


int32_t ATreturn_SNRtotal() {
  return ATSNRtotal;
}


uint8_t ATreturn_ATtype() {
  return ATType;
}


uint8_t ATreturn_SegmentSize() {
  return ATSegmentSize;
}


uint16_t ATarrayCRC(uint8_t *buffer, uint32_t size, uint16_t startvalue) {
  uint32_t index;
  uint16_t libraryCRC;
  uint8_t j;

  libraryCRC = startvalue;  //start value for CRC16

  for (index = 0; index < size; index++) {
    libraryCRC ^= (((uint16_t)buffer[index]) << 8);
    for (j = 0; j < 8; j++) {
      if (libraryCRC & 0x8000)
        libraryCRC = (libraryCRC << 1) ^ 0x1021;
      else
        libraryCRC <<= 1;
    }
  }

  return libraryCRC;
}


void ATprintSourceArrayDetails() {
  uint16_t tempnumber;


#ifdef ENABLEMONITOR
  Monitorport.print(F("Source array length is "));
  Monitorport.print(ATSourceArrayLength);
  Monitorport.println(F(" bytes"));
  Monitorport.print(F("Source array CRC is 0x"));
  Monitorport.println(ATSourceArrayCRC, HEX);
  Monitorport.print(F("Number segments "));
  tempnumber = ATSourceArrayLength / ATSegmentSize;
  if ((ATSegmentSize * tempnumber) < ATSourceArrayLength) {
    tempnumber++;
  }
  Monitorport.println(tempnumber);
#endif
}


void ATprintDestinationArrayDetails() {
#ifdef ENABLEMONITOR
  Monitorport.print(F("Destination array length "));
  Monitorport.print(ATDestinationArrayLength);
  Monitorport.println(F(" bytes"));
  Monitorport.print(F("Source array length "));
  Monitorport.print(ATSourceArrayLength);
  Monitorport.println(F(" bytes"));

  if (ATDestinationArrayLength != ATSourceArrayLength) {
    Monitorport.println(F("ERROR - array lengths do not match"));
  } else {
    Monitorport.println(F("Array lengths match"));
  }

  Monitorport.print(F("Destination array CRC is 0x"));
  Monitorport.println(ATDestinationArrayCRC, HEX);
  Monitorport.print(F("Source array CRC is 0x"));
  Monitorport.println(ATSourceArrayCRC, HEX);

  if (ATDestinationArrayCRC != ATSourceArrayCRC) {
    Monitorport.println(F("ERROR - array CRCs do not match"));
  } else {
    Monitorport.println(F("Array CRCs match"));
  }
#endif
}


//************************************************
//Common functions
//************************************************

void ATprintArrayHEX(uint8_t *buff, uint32_t len) {
  ATUNUSED(buff);
  ATUNUSED(len);

#ifdef ENABLEMONITOR
  uint32_t index, buffdata;

  for (index = 0; index < len; index++) {
    buffdata = buff[index];
    if (buffdata < 16) {
      Monitorport.print(F("0"));
    }
    Monitorport.print(buffdata, HEX);
    Monitorport.print(F(" "));
  }
#endif
}


void ATprintheader(uint8_t *hdr, uint8_t hdrsize) {
  ATUNUSED(hdr);
  ATUNUSED(hdrsize);

#ifdef ENABLEMONITOR
  Monitorport.print(F("HeaderBytes,"));
  Monitorport.print(hdrsize);
  Monitorport.print(F(" "));
  ATprintArrayHEX(hdr, hdrsize);
#endif
}


void ATprintPacketDetails() {

  ATPacketRSSI = LoRa.readPacketRSSI();
  ATPacketSNR = LoRa.readPacketSNR();
  Monitorport.print(F(" RSSI,"));
  Monitorport.print(ATPacketRSSI);
  Monitorport.print(F("dBm"));
  Monitorport.print(F(",SNR,"));
  Monitorport.print(ATPacketSNR);
  Monitorport.print(F("dBm,Segments,"));
  Monitorport.print(ATReceivedSegments);
  Monitorport.print(F(",ATRXOK,"));
  Monitorport.print(ATRXOK);
  Monitorport.print(F(",ATRXErrors,"));
  Monitorport.print(ATRXErrors);
  Monitorport.print(F(" RX"));
}


uint16_t ATestimatePacketTime(uint8_t segsize, uint8_t sf, uint8_t bw) {
  //estimate time for packet size passed. Can be used to set RX and TX timeouts relative to max packet size used

  uint32_t tempBandwidth;
  float symbolTime;
  uint16_t numberread;
  float symbols;
  uint16_t pkttime;

  tempBandwidth = LoRa.returnBandwidth(bw);
  symbolTime = LoRa.calcSymbolTime(tempBandwidth, sf);
  numberread = pgm_read_word_near(Symbols127Bytes + (sf - 5));
  symbols = ((((float)(segsize + 10) / 127)) * numberread) + 12;  //add 12 bytes for preamble, 10 bytes for header and reliable additions
  pkttime = symbols * (float)symbolTime;

#ifdef PRINTPACKETCALC
  Monitorport.print(F("PacketSize "));
  Monitorport.println(segsize);
  Monitorport.print(F("Symbols for 127 Bytes "));
  Monitorport.println(numberread);
  Monitorport.print(F("SymbolTime "));
  Monitorport.print(symbolTime, 3);
  Monitorport.println(F("mS"));
  Monitorport.print(F("Symbols for packet size "));
  Monitorport.println(symbols);
  Monitorport.print(F("Packet time "));
  Monitorport.print(pkttime);
  Monitorport.println(F("mS"));
#endif

  return pkttime;
}


void ATprintReliableStatus() {

#ifdef ENABLEMONITOR

  uint8_t reliableErrors = LoRa.readReliableErrors();
  uint8_t reliableFlags = LoRa.readReliableFlags();

  if (bitRead(reliableErrors, ReliableCRCError)) {
    Monitorport.print(F(",ReliableCRCError"));
  }

  if (bitRead(reliableErrors, ReliableIDError)) {
    Monitorport.print(F(",ReliableIDError"));
  }

  if (bitRead(reliableErrors, ReliableSizeError)) {
    Monitorport.print(F(",ReliableSizeError"));
  }

  if (bitRead(reliableErrors, ReliableACKError)) {
    Monitorport.print(F(",NoReliableACK"));
  }

  if (bitRead(reliableErrors, ReliableTimeout)) {
    Monitorport.print(F(",ReliableTimeout"));
  }

  if (bitRead(reliableFlags, ReliableACKSent)) {
    Monitorport.print(F(",ACKsent"));
  }

  if (bitRead(reliableFlags, ReliableACKReceived)) {
    Monitorport.print(F(",ACKreceived"));
  }
#endif
}
