/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 14/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a modified version of Arducam 'ArduCAM_ESP32_Capture2SD' program that is 
  part of the core for the ArduCAM ESP32UNO board. The modifications allow the SD card on the EPS32 to 
  be used in MMC mode and then transfer the image file saved on the SD card to be transferred via LoRa
  to a receiver NodeMCU ESP32s using program 

  Its necessary to modify the ArduCAM file 'memorysaver.h' as per the instructions for the ArduCAM ESP32S
  UNO core installtion. This ESP32 version of the ArduCAM library is not compatible with other versions of
  the ArduCAM library. 
  
  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

// ArduCAM demo (C)2017 Lee
// Web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions
// of the library with a supported camera modules, and can run on ArduCAM ESP32 UNO platform.
// This demo was made for Omnivision  2MP/5MP sensor.
// It will run the ArduCAM ESP32 2MP/5MP as a real 2MP/5MP digital camera, provide both JPEG capture.
// The demo sketch will do the following tasks:
// 1. Set the sensor to JPEG mode.
// 2. Capture and buffer the image to FIFO every 5 seconds
// 3. Store the image to Micro SD/TF card with JPEG format in sequential.
// 4. Resolution can be changed by myCAM.set_JPEG_size() function.
// This program requires the ArduCAM V4.0.0 (or later) library
// and use Arduino IDE 1.8.1 compiler or above

#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
#include <SD_MMC.h>
#include "FS.h"
#include <ArduCAM.h>                  //get the library here https://github.com/ArduCAM/ArduCAM_ESP32S_UNO
#include "memorysaver.h"
#if !(defined ESP32 )
#error Please select the ArduCAM ESP32 UNO board in the Tools/Board
#endif

//This demo can work on OV2640_MINI_2MP/OV5640_MINI_5MP_PLUS/OV5642_MINI_5MP_PLUS/OV5642_MINI_5MP_PLUS/
//OV5642_MINI_5MP_BIT_ROTATION_FIXED/ ARDUCAM_SHIELD_V2 platform.
#if !(defined (OV2640_MINI_2MP)|| defined (OV2640_MINI_2MP_PLUS)||defined (OV5640_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP_PLUS) \
    || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) \
    ||(defined (ARDUCAM_SHIELD_V2) && (defined (OV2640_CAM) || defined (OV5640_CAM) || defined (OV5642_CAM))))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

char pname[20];
byte buf[256];

static int i = 0;
static int k = 0;
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
bool is_header = false;

#if defined (OV2640_MINI_2MP)|| defined (OV2640_MINI_2MP_PLUS) || defined (OV2640_CAM)
ArduCAM myCAM(OV2640, CS);
#elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
ArduCAM myCAM(OV5640, CS);
#elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
ArduCAM myCAM(OV5642, CS);
#endif

//Additions for LoRa *************************************************************************************************
#include <SX127XLT.h>                        //get the library here > https://github.com/StuartsProjects/SX12XX-LoRa
#include <ProgramLT_Definitions.h>
#include "Settings.h"                        //LoRa settings etc.

SX127XLT LoRa;                               //create an SX127XLT library instance called LoRa, required by SDtransfer.h

#define ENABLEMONITOR                        //enable monitor prints
#define PRINTSEGMENTNUM                      //enable this define to print segment numbers 
#define ENABLEFILECRC                        //enable this define to uses and show file CRCs
//#define DISABLEPAYLOADCRC                  //enable this define if you want to disable payload CRC checking

#include "MMClibrary.h"                      //local library of SD functions
#include "MMCtransfer.h"                     //local library of data transfer functions
//*******************************************************************************************************



void loop()
{
  uint32_t filelength;
  
  sprintf((char*)pname, "/%05d.jpg", k);
  capture2SD(SD_MMC, pname);
  k++;
  
  filelength = SDsendFile(pname, sizeof(pname));

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
  delay(5000);
}


void capture2SD(fs::FS &fs, const char * path) {
  File file ;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Start Capture"));
  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  Serial.println(F("Capture Done."));
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is "));
  Serial.print(length, DEC);
  Serial.println(F(" bytes"));

  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  i = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      file.write(buf, i);
      //Close the file
      file.close();
      Serial.print(F("Image save OK "));
      Serial.println(pname);
      is_header = false;
      i = 0;
    }
    if (is_header == true)
    {
      //Write image data to buffer if not full
      if (i < 256)
        buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        file.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }
}

void setup()
{
  uint8_t vid, pid;
  uint8_t temp;
  static int i = 0;
  
  
  pinMode(2, INPUT_PULLUP);
  digitalWrite(5, HIGH);
  pinMode(5, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();

  SPI.begin();
  if (!SD_MMC.begin("/sdcard", true))         //start MMC card in 1 bit mode
  {
    Serial.println("MMC Card Mount Failed");
    return;
  } else
    Serial.println(F("MMC Card detected!"));
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      Serial.println(F("SPI interface Error!"));
      delay(2);
      continue;
    }
    else
      break;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println(F("No SD card attached"));
    return;
  }
  Serial.print(F("SD Card Type: "));
  if (cardType == CARD_MMC) {
    Serial.println(F("MMC"));
  } else if (cardType == CARD_SD) {
    Serial.println(F("SDSC"));
  } else if (cardType == CARD_SDHC) {
    Serial.println(F("SDHC"));
  } else {
    Serial.println(F("UNKNOWN"));
  }
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);

#if defined (OV2640_MINI_2MP) || defined (OV2640_MINI_2MP_PLUS)|| defined (OV2640_CAM)
  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    Serial.println(F("Can't find OV2640 module!"));
  else
    Serial.println(F("OV2640 detected."));
#elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
  //Check if the camera module type is OV5640
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
  if ((vid != 0x56) || (pid != 0x40))
    Serial.println(F("Can't find OV5640 module!"));
  else
    Serial.println(F("OV5640 detected."));
#elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
  //Check if the camera module type is OV5642
  myCAM.wrSensorReg16_8(0xff, 0x01);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
  if ((vid != 0x56) || (pid != 0x42)) {
    Serial.println(F("Can't find OV5642 module!"));
  }
  else
    Serial.println(F("OV5642 detected."));
#endif
  //Change to JPEG capture mode and initialize the OV2640 module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
#if defined (OV2640_MINI_2MP)|| defined (OV2640_MINI_2MP_PLUS) || defined (OV2640_CAM)
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
#elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.OV5640_set_JPEG_size(OV5640_320x240);
#elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
  myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
  myCAM.OV5642_set_JPEG_size(OV5642_320x240);
#endif
  delay(1000);

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

}
