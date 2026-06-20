/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 17/06/26

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*****************************************************************************************************
These are the Config Settings used by the LoRa Camera Receive. The default constants are assigned at reset
or power up to be named variables that are in a structure, i.e.;

struct settings {
uint32_t Frequency = 434000000;              //Frequency of transmissions in hertz

This process is used since it allows the Serial menus (in ReceiverMenu.h) to change the working frequency 
etc. When the program starts for the first time the values of the variables such as Frequency are saved
in FLASH menory using the Preferances library. Then on subsequent startups the variables used by the
program such as Frequency are loaded from FLASH. In this way permanent changes can be made to values such as 
Frequency via the serial menus without the need for a re-program.

*****************************************************************************************************/

/*
  Issues:
*/

struct settings {
  uint8_t marker1 = 0x55;       //a check byte useful when printing out the settings structure bytes
  uint8_t TXCameraNumber = 1;   //camera number of this transmitter, always 0 for receiver
  uint16_t NetworkID = 0x3210;  //a unique identifier to go out with LoRa packets

  uint32_t Frequency = 434000000;              //Frequency of transmissions in hertz
  uint8_t SpreadingFactor = LORA_SF5;          //LoRa spreading factor
  uint8_t Bandwidth = LORA_BW_500;             //LoRa bandwidth
  uint8_t CodeRate = LORA_CR_4_5;              //LoRa coding rate
  int8_t TXpower = 10;                         //LoRa transmit power in dBm
  uint8_t StandardImageSize = FRAMESIZE_UXGA;  //Standard image size 1600x1200
  uint8_t SentImageSize = FRAMESIZE_QQVGA;     //Reduced image size 160x120
  uint8_t SegmentSize = 243;                   //number of bytes in each segment, 245 is maximum
  uint8_t PicturesToTake = 3;                  //number of pictures to take at each wakeup, only last is sent via LoRa
  uint32_t TimeToSleep = 60;                   //sleep time in seconds
  uint8_t TXActions = 0b00000000;              //TX action flags
  uint8_t RXActions = 0b00000000;              //RX action flags
  uint8_t TXFlags = 0;                         //TXFlags, status of TX
  uint8_t RXFlags = 0;                         //RXFlags, status of RX

  //aditional constants, ones most likly needing to be changed listed first
  uint16_t WakePreAmble = 64;             //length of preamble to send for RX duty cycle wake on camera
  uint8_t ListenCameraNumber = 0;         //number of camera to listen for, 0 = listen for all
  uint8_t MaxRequestedImageAttempts = 3;  //max times to attempt requested image download
  uint32_t RXtimeoutmS = 60000;           //RX timeout for non transfer LoRa use
  uint16_t MenuTimeout = 30000;           //Timeout for serial menu
  uint32_t FreqOffset = 0;                //Offset frequency in hertz for calibration purposes
  uint8_t Optimisation = LDRO_AUTO;       //Low data rate optimisation setting, normally set to auto
  uint32_t RequestedImage;                //image number for download

  bool EnforceBasicSettings = false;     //always reply to transmitter ready broadcast with basic settings
  bool EnforceExtendedSettings = false;  //always reply to transmitter ready broadcast with extended settings

  //Constants for Long range LoRa settings
  uint32_t LRFREQ = 434000000;  //Frequency of transmissions in hertz
  uint32_t LRFREQOFF = 0;       //Offset frequency in hertz for calibration purposes
  uint8_t LRSF = LORA_SF12;     //LoRa spreading factor
  uint8_t LRBW = LORA_BW_125;   //LoRa bandwidth
  uint8_t LRCR = LORA_CR_4_5;   //LoRa coding rate
  uint8_t LROPT = LDRO_AUTO;    //LoRa optimisation
  uint8_t LRTXPOW = 10;         //LoRa transmit power in dBm

  /**************************************
Array transfer definitions
**************************************/

  uint8_t ATSendAttempts = 3;           //number of times to attempt to send an array transfer
  uint8_t ATStartAttempts = 2;          //number of times to start transfer process, i.e. open file on receiver
  uint8_t ATDataSizeMax = 245;          //max number of payload bytes in a packets
  uint16_t ATTXtimeoutmS = 30000;       //timeout of transfer transmissions
  uint32_t ATRXtimeoutmS = 5000;        //RX timeout for actual transfer
  uint16_t ATACKsegtimeoutmS = 1500;    //max time to wait for s send segment ack to be received
  uint16_t ATDuplicatedelaymS = 25;     //delay for when a duplicate segment is received
  uint16_t ATACKdelaymS = 25;           //delay when sending an ack during the transfer process, allows time for transmitter to be ready
  uint8_t ATHeaderSizeMax = 14;         //max size of header in bytes, minimum size is 7 bytes
  uint16_t ATACKopentimeoutmS = 1500;   //mS to wait for receiving an ACK before re-trying transmit file open
  uint16_t ATACKclosetimeoutmS = 1500;  //mS to wait for receiving an ACK before re-trying transmit file close
  uint16_t ATACKdelaystartendmS = 25;   //mS delay before ack sent at array start write and end write
  uint32_t ATFunctionDelaymS = 0;       //delay between functions such as open file, send segments etc
  uint32_t ATNoAckCountLimit = 250;     //if no NoAckCount exceeds this value - restart transfer
  uint8_t marker2 = 0xAA;               //a check byte useful when printing out the settings structure bytes
} __attribute__((packed, aligned(1)));  //remove structure padding to ensure compatibility between 8bit and 32bit micros;

struct settings var;      //contains operational variables
struct settings vartemp;  //temp store of variables for making chnages

const uint8_t TXsavestore = 0;   //bit number for TXActions byte to save settings in store, 1 = save settings to store
const uint8_t TXerasestore = 1;  //bit number for TXActions byte to store erase, 1 = erase store settings

const uint8_t TXRequestedimage = 3;   //bit number for TXFlags byte when a requested image download has beed requested
const uint8_t TXcameraerror = 2;      //bit number for TXFlags byte when TX detects a camera error
const uint8_t TXSDerror = 1;          //bit number for TXFlags byte when TX detects an SD error
const uint8_t TXFlashStoreValid = 0;  //bit number for TXFlags byte when flash store is valid

bool SENDACK2 = false;  //set to true if ATReadyACK2 to be sent
bool SENDACK3 = false;  //set to true if ATReadyACK3 to be sent

const uint8_t REQUESTEDIMAGE = 0xFE;
const uint8_t FILEERROR = 0xFF;

const uint8_t brightness = 32;  //brightness of RGB LEDs
