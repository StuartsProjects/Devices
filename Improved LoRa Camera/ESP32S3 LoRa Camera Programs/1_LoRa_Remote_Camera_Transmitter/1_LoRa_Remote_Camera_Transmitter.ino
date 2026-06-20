/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/06/26

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  This program is for ESP32S3 Camera boards. Currently supported are some custom PCBs that use the ESP32S3
  Camera development board. 
  
  The program starts, initialises the camera, SD card and LoRa device then takes a serias of pictures. 
  If there is a LoRa receiver in range then an image is transfered to the receiver via LoRa.
  
  The camera is then put into standby mode by a register write that will only work on OV3660 and OV5640
  cameras, an OV2640 camera will not go into standby.

  The ESP32S3 is then put into deep sleep for a number of seconds.

  The current with SD card and Camera in deep sleep on the ESP32S3 LoRa Camera Board 050426 is circa 0.65mA.
  Running current on that board with the OV3660 camera active is circa 125mA.

  Use program 2_LoRa_Remote_Camera_Receiver to receive the images.

  Uses Arduino IDE 2 and ESP32 Core 3.3.8, do not use other cores there can be issues with the camera images.
  Be sure to set the PSRAM to 'Enabled' and to the correct type, QSPI or OPI, for your ESP32S3 boards.

  Serial monitor baud rate is set at 115200. Use the CH343 UART port for programming.
*******************************************************************************************************/

/*
  Issues:
 */

//#define DEBUG               //Enable debug prints in this sketch
#define Monitorport Serial  //default port for serial prints

#include "ESP32S3_LoRa_Mini_Camera_Board_120426.h"
//#include "ESP32S3_LoRa_Camera_Board_050426.h"

#include "esp_camera.h"
camera_config_t config;
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"

#include <SX126XLT.h>  //SX12XX-LoRa library get it here > https://github.com/StuartsProjects/SX12XX-LoRa
SX126XLT LoRa;

#include "ConfigSettings.h"     //LoRa and program settings and variables
#include "TransferFunctions.h"  //LoRa library file for array transmit and receive
#include "StoreFunctions.h"     //Library file of functions for saving variables to store
#include <SPI.h>                //for LoRa module
#include <SD_MMC.h>             //for SD card in MMC mode

bool SDFOUND = false;      //set true if SD OK on intialisation
bool LORAFOUND = false;    //set true LoRa OK on intialisation
bool CAMERAFOUND = false;  //set to true if Camera initially detected OK
bool PSRAMFOUND;

sensor_t *sensor;                  //referance for camera
camera_fb_t *fb = NULL;            //define camera frame buffer
#define uS_TO_S_FACTOR 1000000ULL  //conversion factor for micro seconds to seconds
RTC_DATA_ATTR uint16_t imageCount = 1;

char filename[16];         //filename for saving to SD
uint32_t available_PSRAM;  //amount of PSRAM found
uint32_t filesize;
uint16_t filecrc;
uint16_t arrayCRC;
uint8_t *PSRAMptr;  //create a global pointer to the array to send, so all functions have accessuint32_t available_PSRAM;
uint32_t fbsize;    //size of fram buffer = picture size
uint32_t allocated_PSRAM;

uint8_t RXPayloadL;
uint8_t TXPayloadL;
uint8_t TXPacketL;
uint8_t RXPacketL;
uint8_t RXPacketType;
uint8_t Flags;
uint16_t PayloadCRC;     //CRC of the payload
uint8_t RXCameraNumber;  //camera number received in settings packet
uint16_t SegmentTime;    //estimated time mS for a segment


void loop() {
  uint8_t index;
  RXCameraNumber = 0;

  if (LORAFOUND) {
    broadcast_Ready_Settings(1);
    Monitorport.println(F("Sent ready packet"));
    Monitorport.println(F("Wait ACK"));
    listen_Packet(2500);  //wait 2500mS for ack to sent ready
  }

  if (CAMERAFOUND)  //Camera is available, so take pictures
  {
    fb = NULL;
    sensor->set_framesize(sensor, (framesize_t)var.StandardImageSize);
    fb = esp_camera_fb_get();  //take a picture
    esp_camera_fb_return(fb);  //release image buffer
    delay(100);

    for (index = 1; index <= var.PicturesToTake; index++) {
      //sprintf(filename, "/%d.jpg", imageCount);  //create filename in buffer, replaces %d with imageCount
      build_filename(filename, var.TXCameraNumber, imageCount);

      if (pictureTakeSend(filename, sizeof(filename), (var.PicturesToTake - index))) {
        imageCount++;
        delay(200);
      } else {
        Monitorport.println("ERROR - No picture taken");
        Monitorport.println();
        delay(200);
      }
    }
  } else {
    Monitorport.println(F("ERROR - No Camera, No picture"));
  }

  if ((var.RequestedImage > 0) && (RXCameraNumber == var.TXCameraNumber)) {
    Monitorport.print(F("RequestedImage "));
    Monitorport.println(var.RequestedImage);
    if (load_Send_Image()) {
      Monitorport.print(var.RequestedImage);
      Monitorport.println(F(" sent"));
    } else {
      Monitorport.print(var.RequestedImage);
      Monitorport.println(F(" failed"));
    }
  }

  if (CAMERAFOUND) {
    Monitorport.println(F("Camera to idle"));
    delay(50);
    sensor->set_reg(sensor, 0x3008, 0xFF, 0x42);  //OV3660 or OV5640 camera to standby
    delay(200);
  }

  if (LORAFOUND) {
    Monitorport.println(F("LoRa to sleep"));
    LoRa.setSleep(CONFIGURATION_RETENTION);
    delay(100);
  }

  rgboff();

  Monitorport.print(F("Start Deep sleep "));
  Monitorport.print(var.TimeToSleep);
  Monitorport.println(F(" Secs"));
  Monitorport.println();
  Monitorport.println();
  Monitorport.flush();
  esp_sleep_enable_timer_wakeup(var.TimeToSleep * uS_TO_S_FACTOR);
  esp_deep_sleep_start();

  Monitorport.println(F("Awake !!!!!!!!!! should not see this"));
}


void build_filename(char *buff, uint8_t CameraNumber, uint16_t imagecount) {
  buff[0] = '/';
  buff[1] = CameraNumber / 100 + '0';
  buff[2] = ((CameraNumber % 100) / 10) + '0';
  buff[3] = CameraNumber % 10 + '0';

  buff[4] = imagecount / 10000 + '0';
  buff[5] = ((imagecount % 10000) / 1000) + '0';
  buff[6] = ((imagecount % 1000) / 100) + '0';
  buff[7] = ((imagecount % 100) / 10) + '0';
  buff[8] = imagecount % 10 + '0';

  buff[9] = '.';
  buff[10] = 'j';
  buff[11] = 'p';
  buff[12] = 'g';
  buff[13] = 0;
}


bool load_Send_Image() {
  uint32_t index;

  //sprintf(filename, "/%d.jpg", var.RequestedImage);  //create filename in buffer, replaces %d with RequestedImage
  build_filename(filename, var.TXCameraNumber, var.RequestedImage);

  Monitorport.print("RequestedImage - Opening SD file ");
  Monitorport.print(filename);

  if (!SD_MMC.exists(filename)) {
    Monitorport.println();
    Monitorport.print("ERROR ");
    Monitorport.print(filename);
    Monitorport.println(" not found on SD card");
    ERRORSDon();
    var.RequestedImage = 0;
    bitSet(var.TXFlags, TXLoadFileError);  //set bit in TXFlags indicating file error

    ATsendArray(PSRAMptr, filesize, filename, sizeof(filename), var.SegmentSize, FILEERROR, var.TXFlags);

    delay(1000);
    return false;
  }

  File dataFile = SD_MMC.open(filename);

  filesize = dataFile.size();
  Monitorport.print(" ");
  Monitorport.print(filesize);
  Monitorport.println(" bytes");
  dataFile.seek(0);

  for (index = 0; index < filesize; index++) {
    PSRAMptr[index] = dataFile.read();
  }

  Monitorport.print(index + 1);
  Monitorport.println(" bytes copied to PSRAM");
  Monitorport.print(F("Sending image with LoRa "));
  Monitorport.println(filename);
  LoRa.setupLoRa(var.Frequency, var.FreqOffset, var.SpreadingFactor, var.Bandwidth, var.CodeRate, var.Optimisation);  //configure var.Frequency and LoRa settings
  delay(10);

  if (ATsendArray(PSRAMptr, filesize, filename, sizeof(filename), var.SegmentSize, REQUESTEDIMAGE, 0))  //pass array pointer and length across to LoRa send function
  {
    Monitorport.println(F("Send finished"));
  } else {
    Monitorport.println(F("Send failed"));
    return false;
  }
  return true;
}


void broadcast_Ready_Settings(uint8_t broadcasts) {
  uint8_t index;

  LoRa.setupLoRa(var.LRFREQ, var.LRFREQOFF, var.LRSF, var.LRBW, var.LRCR, var.LROPT);  //configure long range var.Frequency and LoRa settings
  LoRa.startWriteSXBuffer(0);                                                          //start the write at SX12XX internal buffer location startaddr
  LoRa.writeUint8(ATReady);                                                            //packet type
  LoRa.writeUint8(var.TXCameraNumber);
  LoRa.writeUint8(var.TXFlags);  //contains status bits for TX
  LoRa.writeUint32(var.Frequency);
  LoRa.writeUint8(var.SpreadingFactor);
  LoRa.writeUint8(var.Bandwidth);
  LoRa.writeUint8(var.CodeRate);
  TXPayloadL = LoRa.endWriteSXBuffer();  //closes packet write and returns the length of the payload to send

#ifdef DEBUG
  Monitorport.print(F("Broadcast_Ready_Settings packet length "));
  Monitorport.println(TXPayloadL + 4);
#endif

  for (index = 0; index < broadcasts; index++) {
    TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, var.NetworkID, 10000, var.TXpower, WAIT_TX);
  }
}


bool listen_Packet(uint32_t waittime) {
  bool gotACK = false;
  Monitorport.print(F("Listen for packet "));
  Monitorport.print(waittime);
  Monitorport.println(F("mS"));
  uint32_t startwaitmS = millis();
  LoRa.receiveSXReliable(0, var.NetworkID, 0, NO_WAIT);

  do {
    if (digitalRead(LORA_DIO1)) {
      RXPacketL = LoRa.readRXPacketL();
      if (RXPacketL) {
        Monitorport.println(F("LoRa Packet received"));
        LoRa.startReadSXBuffer(0);
        RXPacketType = LoRa.readUint8();
        RXCameraNumber = LoRa.readUint8();
        var.TXActions = LoRa.readUint8();
        LoRa.endReadSXBuffer();
        Monitorport.print(F("RXPacketL "));
        Monitorport.println(RXPacketL);
        Monitorport.print(F("RXPacketType 0x"));
        Monitorport.println(RXPacketType, HEX);
        Monitorport.print(F("For Camera Number "));
        Monitorport.println(RXCameraNumber);
        Monitorport.print(F("TXActions 0x"));
        Monitorport.println(var.TXActions);
        Monitorport.println();

        if ((RXPacketType == ATReadyACK1) || (RXPacketType == ATReadyACK2) || (RXPacketType == ATReadyACK3)) {
          if ((RXCameraNumber == 0) || (RXCameraNumber == var.TXCameraNumber))  //if cameranumber is 0 or numbers match process packet
          {
            Monitorport.println(F("Camera match"));
            process_Packet(RXPacketType);
            return true;
          } else {
            Monitorport.println(F("Camera missmatch"));
            startwaitmS = millis();
            var.TXActions = 0;  //if no camera match clear any TXActions
            LoRa.receiveSXReliable(0, var.NetworkID, 0, NO_WAIT);
          }
        } else {
          Monitorport.println(F("No actionable packet received"));
        }
      } else {
        Monitorport.print(F("Packet ERROR"));
        startwaitmS = millis();
        LoRa.receiveSXReliable(0, var.NetworkID, 0, NO_WAIT);
      }
    }

  } while ((millis() < startwaitmS + waittime));
  Monitorport.println(F("No packet received"));
  return false;
}


bool process_Packet(uint8_t type) {
  //first 3 bytes of pacet, packet type, camera number and TXActions already read
  Monitorport.print(F("Process packet type 0x"));
  Monitorport.println(type, HEX);

  if (type == ATReadyACK1) {
    Monitorport.println(F("ATReadyACK1 received"));
    Monitorport.println();
    process_Actions();
    print_Settings();
    Monitorport.println();
    return true;
  }

  if (type == ATReadyACK2) {
    Monitorport.println(F("ATReadyACK2 received"));
    LoRa.startReadSXBuffer(3);  //start buffer read at location 3, already read packet type, camera number and TXActions
    var.Frequency = LoRa.readUint32();
    var.SpreadingFactor = LoRa.readUint8();
    var.Bandwidth = LoRa.readUint8();
    var.CodeRate = LoRa.readUint8();
    LoRa.endReadSXBuffer();
    Monitorport.print(F("Received settings for "));
    Monitorport.println(RXCameraNumber);
    process_Actions();
    Monitorport.println();
    print_Settings();
    Monitorport.println();
    return true;
  }

  if (type == ATReadyACK3) {
    Monitorport.println(F("ATReadyACK3 received"));
    LoRa.startReadSXBuffer(3);  //start buffer read at location 3, already read packet type, camera number and TXActions
    var.Frequency = LoRa.readUint32();
    var.SpreadingFactor = LoRa.readUint8();
    var.Bandwidth = LoRa.readUint8();
    var.CodeRate = LoRa.readUint8();
    var.TXpower = LoRa.readInt8();
    var.StandardImageSize = LoRa.readUint8();
    var.SentImageSize = LoRa.readUint8();
    var.SegmentSize = LoRa.readUint8();
    var.PicturesToTake = LoRa.readUint8();
    var.TimeToSleep = LoRa.readUint32();
    var.RequestedImage = LoRa.readUint32();
    LoRa.endReadSXBuffer();
    Monitorport.print(F("Received settings for "));
    Monitorport.println(RXCameraNumber);
    check_Image_Sizes();
    process_Actions();
    Monitorport.println();
    print_Settings();
    Monitorport.println();
    return true;
  }

  Monitorport.println(F("Packet Not recognised"));
  LoRa.receiveSXReliable(0, var.NetworkID, var.RXtimeoutmS, NO_WAIT);
  return false;
}


void process_Actions() {
  //check if there are any actions to process, carry them out and clear TXActions
  //dont process TXActions if RX is listening for camera 0, i.e. all cameras

  if (RXCameraNumber == 0) {
    var.TXActions = 0;
    return;
  }

  if (var.TXActions) {

    if (bitRead(var.TXActions, TXerasestore)) {
      erase_store();
      bitClear(var.TXActions, TXerasestore);  //clear the request
    }

    if (bitRead(var.TXActions, TXsavestore)) {
      copy_var_store();
      bitClear(var.TXActions, TXsavestore);  //clear the request
    }
  }

  return;
}


void check_Image_Sizes() {
  if (var.StandardImageSize > FRAMESIZE_5MP) {
    Monitorport.println(F("StandardImageSize Image size error "));
    Monitorport.println(var.StandardImageSize);
    var.StandardImageSize = FRAMESIZE_VGA;
    Monitorport.println(F("Now set to "));
    Monitorport.println(var.StandardImageSize);
  }

  if (var.SentImageSize > FRAMESIZE_5MP) {
    Monitorport.println(F("SentImageSize Image size error "));
    Monitorport.println(var.SentImageSize);
    var.SentImageSize = FRAMESIZE_QQVGA;
    ;
    Monitorport.println(F("Now set to "));
    Monitorport.println(var.SentImageSize);
  }
}


void print_Settings() {
  Monitorport.println();
  Monitorport.println(F("Transmitter Settings"));
  Monitorport.println(F("--------------------"));

  Monitorport.print(F("This CameraNumber "));
  Monitorport.println(var.TXCameraNumber);

  Monitorport.print(F("TXActions 0x"));
  Monitorport.println(var.TXActions, HEX);

  Monitorport.print(F("Frequency "));
  Monitorport.print(var.Frequency);
  Monitorport.println(F("Hz"));

  Monitorport.print(F("FreqOffset "));
  Monitorport.print(var.FreqOffset);
  Monitorport.println(F("Hz"));

  Monitorport.print(F("SpreadingFactor "));
  Monitorport.println(var.SpreadingFactor);

  Monitorport.print(F("Bandwidth "));
  Monitorport.print(LoRa.returnBandwidth(var.Bandwidth));
  Monitorport.println(F("Hz"));

  Monitorport.print(F("CodeRate 4:"));
  Monitorport.println(var.CodeRate + 4);

  Monitorport.print(F("TXpower "));
  Monitorport.print(var.TXpower);
  Monitorport.println(F("dBm"));

  Monitorport.print(F("SegmentSize "));
  Monitorport.print(var.SegmentSize);
  SegmentTime = ATestimatePacketTime(var.SegmentSize, var.SpreadingFactor, var.Bandwidth);
  Monitorport.print(F(" ("));
  Monitorport.print(SegmentTime);
  Monitorport.println(F("mS)"));

  Monitorport.print(F("StandardImageSize "));
  print_ImageSize(var.StandardImageSize);
  Monitorport.println();

  Monitorport.print(F("SentImageSize "));
  print_ImageSize(var.SentImageSize);
  Monitorport.println();

  Monitorport.print(F("TimeToSleep "));
  Monitorport.print(var.TimeToSleep);
  Monitorport.println(F(" Secs"));

  Monitorport.print(F("PicturesToTake "));
  Monitorport.println(var.PicturesToTake);

  Monitorport.print(F("RequestedImage "));
  Monitorport.println(var.RequestedImage);

  Monitorport.println();
}


bool pictureTakeSend(char *Name, uint8_t namesize, uint8_t picsleft) {
  fb = NULL;
  fb = esp_camera_fb_get();  //take a picture
  esp_camera_fb_return(fb);  //release image buffer
  delay(100);

  if (picsleft != 0) {
    sensor->set_framesize(sensor, (framesize_t)var.StandardImageSize);
    Monitorport.print(F("StandardImageSize "));
    print_ImageSize(var.StandardImageSize);
    Monitorport.println();
  } else {
    sensor->set_framesize(sensor, (framesize_t)var.SentImageSize);
    fb = esp_camera_fb_get();  //need to take a null picture for set_framesize to take effect
    esp_camera_fb_return(fb);  //release image buffer
    print_ImageSize(var.SentImageSize);
    Monitorport.println();
  }

  delay(100);
  esp_camera_fb_get();  //take the picture

  if (!fb) {
    Monitorport.println("ERROR - Failed to get camera frame buffer");
    esp_camera_fb_return(fb);  //release image buffer
    ERRORCamon();
    delay(1000);
    return false;
  }

  PSRAMptr = fb->buf;  //put pointer to frame buffer in global variable
  fbsize = fb->len;    //save image size in global variable

  Monitorport.print("Picture taken, Image size ");
  Monitorport.print(fbsize);
  Monitorport.println(" bytes");

  if (SDFOUND) {
    if (writeFile(Name, PSRAMptr, fbsize))  //save picture to file
    {
      Monitorport.println(F("File saved OK"));
    } else {
      Monitorport.println(F("ERROR - File not saved"));
      ERRORSDon();
      delay(1000);
    }
  } else {
    Monitorport.println(F("ERROR - No SD card"));
    ERRORSDon();
    delay(1000);
  }

  Monitorport.print("Images left in sequence ");
  Monitorport.println(picsleft);

  if (picsleft == 0) {  //if is last picture in sequence
    if (LORAFOUND) {
      arrayCRC = ATarrayCRC(fb->buf, fb->len, 0xFFFF);
      Monitorport.print(F("PSRAM image array CRC is 0x"));
      Monitorport.println(arrayCRC, HEX);
      Monitorport.print(F("Sending image with LoRa "));
      Monitorport.println(Name);
      Monitorport.println();

      LoRa.setupLoRa(var.Frequency, var.FreqOffset, var.SpreadingFactor, var.Bandwidth, var.CodeRate, var.Optimisation);  //configure var.Frequency and LoRa settings
      delay(10);

      if (ATsendArray(fb->buf, fb->len, Name, namesize, var.SegmentSize, var.SentImageSize, 0))  //pass array pointer and length across to LoRa send function
      {
        Monitorport.println(F("Send finished"));
      } else {
        Monitorport.println(F("Send failed"));
      }
    } else {
      Monitorport.println(F("No LoRa Device for image transfer"));
    }
  }

  esp_camera_fb_return(fb);  //release image buffer
  return true;
}


void print_ImageSize(uint8_t imagesize) {
  switch (imagesize) {
    case FRAMESIZE_96X96:
      Monitorport.print(F("0 FRAMESIZE_96X96 96x96"));
      break;

    case FRAMESIZE_QQVGA:
      Monitorport.print(F("1 FRAMESIZE_QQVGA 160x120"));
      break;

    case FRAMESIZE_128X128:
      Monitorport.print(F("2 FRAMESIZE_128X128 128x128"));
      break;

    case FRAMESIZE_QCIF:
      Monitorport.print(F("3 FRAMESIZE_QCIF 176x144"));
      break;

    case FRAMESIZE_HQVGA:
      Monitorport.print(F("4 FRAMESIZE_HQVGA 240x176"));
      break;

    case FRAMESIZE_240X240:
      Monitorport.print(F("5 FRAMESIZE_240x240 240x240"));
      break;

    case FRAMESIZE_QVGA:
      Monitorport.print(F("6 FRAMESIZE_QVGA 320x240"));
      break;

    case FRAMESIZE_320X320:
      Monitorport.print(F("7 FRAMESIZE_320X320 320X320"));
      break;

    case FRAMESIZE_CIF:
      Monitorport.print(F("8 FRAMESIZE_CIF 400x296"));
      break;

    case FRAMESIZE_HVGA:
      Monitorport.print(F("9 FRAMESIZE_HVGA 480x320"));
      break;

    case FRAMESIZE_VGA:
      Monitorport.print(F("10 FRAMESIZE_VGA 640x480"));
      break;

    case FRAMESIZE_SVGA:
      Monitorport.print(F("11 FRAMESIZE_SVGA 800x600"));
      break;

    case FRAMESIZE_XGA:
      Monitorport.print(F("12 FRAMESIZE_XGA 1024x768"));
      break;

    case FRAMESIZE_HD:
      Monitorport.print(F("13 FRAMESIZE_HD 1280x720"));
      break;

    case FRAMESIZE_SXGA:
      Monitorport.print(F("14 FRAMESIZE_SXGA 1280x1024"));
      break;

    case FRAMESIZE_UXGA:
      Monitorport.print(F("15 FRAMESIZE_UXGA 1600x1200"));
      break;

      //3MP Sensors

    case FRAMESIZE_FHD:
      Monitorport.print(F("16 FRAMESIZE_FHD 1920x1080"));
      break;

    case FRAMESIZE_P_HD:
      Monitorport.print(F("17 FRAMESIZE_P_HD 720x1280"));
      break;

    case FRAMESIZE_P_3MP:
      Monitorport.print(F("18 FRAMESIZE_P_3MP 864x1536"));
      break;

    case FRAMESIZE_QXGA:
      Monitorport.print(F("19 FRAMESIZE_QXGA 2048x1536"));
      break;

      //5MP Sensors

    case FRAMESIZE_QHD:
      Monitorport.print(F("20 FRAMESIZE_QHD 2560x1440"));
      break;

    case FRAMESIZE_WQXGA:
      Monitorport.print(F("21 FRAMESIZE_WQXGA 2560x1600"));
      break;

    case FRAMESIZE_P_FHD:
      Monitorport.print(F("22 FRAMESIZE_P_FHD 1080x1920"));
      break;

    case FRAMESIZE_QSXGA:
      Monitorport.print(F("23 FRAMESIZE_QSXGA 2560x1920"));
      break;

    case FRAMESIZE_5MP:
      Monitorport.print(F("24 FRAMESIZE_5MP 2592x1944"));
      break;

    case FRAMESIZE_INVALID:
      Monitorport.print(F("25 FRAMESIZE_INVALID"));
      break;

    case REQUESTEDIMAGE:
      Monitorport.print(F("No size - REQUESTEDIMAGE"));
      break;

    case FILEERROR:
      Monitorport.print(F("FILE ERROR"));
      break;

    default:
      Monitorport.print(imagesize);
      Monitorport.print(F(" FRAMESIZE_INVALID"));
      break;
  }
}


bool writeFile(const char *path, uint8_t *data, size_t len) {
  Monitorport.printf("Writing file to SD %s\r\n", path);

  File file = SD_MMC.open(path, FILE_WRITE);

  if (!file) {
    Monitorport.println("ERROR - Failed to open file for writing");
    ERRORSDon();
    delay(1000);
    return false;
  }

  if (file.write(data, len) == len) {
    return true;
  } else {
    ERRORSDon();
    delay(1000);
    return false;
  }
  file.close();
}


bool camera_init() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  //config.frame_size = FRAMESIZE_XGA;
  config.frame_size = FRAMESIZE_QXGA;  //do intial setup with largest image size
  //config.frame_size = var.StandardImageSize;
  config.pixel_format = PIXFORMAT_JPEG;
  //config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);  //camera init

  if (err != ESP_OK) {
    Monitorport.printf("ERROR - Camera init failed with error 0x%x", err);
    return false;
  } else {
    Monitorport.println("Camera init OK");
    sensor = esp_camera_sensor_get();      //referance for camera
    sensor->set_hmirror(sensor, HMIRROR);  //0 = disable , 1 = enable
    sensor->set_vflip(sensor, VFLIP);      //0 = disable , 1 = enable
    return true;
  }
}


bool check_PSRAM() {
  if (psramInit()) {
    available_PSRAM = ESP.getFreePsram();
    Monitorport.print("PSRAM available: ");
    Monitorport.print(available_PSRAM);
    Monitorport.println(" bytes");
    allocated_PSRAM = available_PSRAM / 2;  //dont use all PSRAM for array
    Monitorport.print(allocated_PSRAM);
    Monitorport.println(" bytes of PSRAM reserved for images");
    uint8_t *byte_array = (uint8_t *)ps_malloc((allocated_PSRAM) * sizeof(uint8_t));
    PSRAMptr = byte_array;  //save the pointer to byte_array as global pointer
    return true;
  } else {
    Monitorport.println("ERROR - PSRAM not available");
    Monitorport.println();
    return false;
  }
  return false;
}


bool setupMMCSD() {
  SD_MMC.setPins(MMC_SCK, MMC_CMD, MMC_D0);

  if (SD_MMC.begin("/sdcard", true))  //start MMC in 1 bit mode
  {
    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE) {
      Monitorport.println("ERROR - SD card type not recognised");
      return false;
    } else {
      return true;
    }
  } else {
    Monitorport.println("ERROR - MMC SD card mount failed");
    return false;
  }
}


bool setupLoRaDevice() {
  if (!LoRa.begin(LORA_NSS, LORA_RESET, LORA_BUSY, LORA_DIO1, LORA_DEVICE)) {
    return false;
  }
  return true;
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, LEDON);  //Turn on the LED
    delay(delaymS);
    digitalWrite(LED1, LEDOFF);  //Turn off the LED
  }
}


void OKLEDon() {
  rgbLedWrite(RGB_LED, 0, brightness, 0);  //green for program running
}


void ERRORon() {
  rgbLedWrite(RGB_LED, brightness, 0, 0);  //red for LoRa active
}


void Serialon() {
  rgbLedWrite(RGB_LED, brightness / 2, 0, brightness);  //magenta for serial menu active
}


void ERRORSDon() {
  rgbLedWrite(RGB_LED, 0, 0, brightness);  //blue for SD error
}


void ERRORCamon() {
  rgbLedWrite(RGB_LED, brightness, brightness, 0);  //yello for camera error
}


void rgboff() {
  rgbLedWrite(RGB_LED, 0, 0, 0);  //ledoff
}


void setup() {
  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  pinMode(LED1, OUTPUT);
  led_Flash(2, 250);  //flash LED to sugnal wakeup
  OKLEDon();

  Monitorport.begin(115200);
  Monitorport.println();
  Monitorport.println(__FILE__);

  init_store();

  if (check_store())  //check variables in Flash store, if structure valid use them
  {
    Monitorport.println();
    Monitorport.println(F("**** FLASH Store valid ****"));
    copy_store_var();
    Monitorport.println(F("Using FLASH Store settings"));
    Monitorport.println();
    bitSet(var.TXFlags, TXFlashStoreValid);

  } else {
    Monitorport.println();
    Monitorport.println(F("**** FLASH Store not valid ****"));
    Monitorport.println(F("Using program defaults"));
    Monitorport.println();
    bitClear(var.TXFlags, TXFlashStoreValid);
  }

  if (!check_PSRAM()) {
    Monitorport.println("ERROR - PSRAM");
    Monitorport.println();
    Monitorport.println("Program halted");
    ERRORon();
    while (1) {
      ERRORon();
      delay(500);
      rgboff();
      delay(500);
    };
  }

  if (camera_init()) {
    CAMERAFOUND = true;
  } else {
    CAMERAFOUND = false;
    ERRORCamon();
    Monitorport.println("Program halted");
    do {
      led_Flash(1, 25);
    } while (1);
  }

  if (setupMMCSD()) {
    Monitorport.println("SD Card Mount OK");
    SDFOUND = true;
  } else {
    Monitorport.println("ERROR - SD Card Mount Failed");
    SDFOUND = false;
    ERRORSDon();
    delay(2000);
  }

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);

  if (setupLoRaDevice()) {
    Monitorport.println(F("LoRa device OK"));
    LORAFOUND = true;
  } else {
    Monitorport.println("ERROR - LoRa Device Failed");
    ERRORon();
    LORAFOUND = false;
  }

  check_Image_Sizes();
  Monitorport.println();
}
