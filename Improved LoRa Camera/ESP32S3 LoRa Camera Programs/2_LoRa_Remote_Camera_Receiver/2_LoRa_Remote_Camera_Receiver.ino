/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 17/06/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation -  This is a receiver program for files or images transferred via LoRa using program
  1_LoRa_Remote_Camera_Transmitter.

  Uses Arduino IDE 2 and ESP32 Core 3.3.8, do not use other cores there can be issues with the camera images.
  Be sure to set the PSRAM to 'Enabled' and to the correct type, QSPI or OPI, for your ESP32S3 boards.

  Serial monitor baud rate is set at 115200. Use the CH343 UART port for programming.
*******************************************************************************************************/

/*
 Issues:
*/

//#define DEBUG                   //Enable debug prints in this sketch
#define Monitorport Serial  //This definition can be chnaged so that serila prints can be redirected to another serial port

#include <SPI.h>
#include <SD_MMC.h>  //for SD Card in MMC mode

SPIClass disp_SPI(HSPI);
#include <esp_camera.h>  //needed to pick up camera frame size settings

//#include "ESP32S3_LoRa_Camera_Board_150226.h"
#include "ESP32S3_LoRa_Camera_Board_050426.h"

#include <SX126XLT.h>  //SX12XX-LoRa library get it here > https://github.com/StuartsProjects/SX12XX-LoRa
SX126XLT LoRa;         //create an SX126XLT library instance called LoRa
#include "ConfigSettings.h"
#include "TransferFunctions.h"  //Library file of Array Transfer functions
#include "StoreFunctions.h"     //Library file of functions for saving variables to store
#include "ReceiverMenu.h"       //Library file of serial menus for configuring this receiver

#include <JPEGDEC.h>
JPEGDEC jpg;

#include <JPEGDisplay.h>  //get library here >  https://github.com/bitbank2/JPEGDEC/tree/master
#include <JPEGDisplay.inl>
JPEGDisplay jd;

#include <bb_spi_lcd.h>  //get library here >  https://github.com/bitbank2/bb_spi_lcd
BB_SPI_LCD disp;

uint8_t *PSRAMptr;  //create a global pointer to the array to send, so all functions have access
uint32_t available_PSRAM;
uint32_t allocated_PSRAM;
uint32_t StartmS;
uint8_t RXPayloadL;
uint8_t TXPayloadL;
uint8_t RXPacketType;
uint8_t RXPacketL;
uint8_t TXPacketL;
uint32_t EndmS;
uint16_t xcord, ycord;
uint16_t jpgwidth;
uint16_t jpgheight;
uint16_t dwidth;
uint16_t dheight;
uint8_t scale;
uint8_t RXCameraNumber;
uint16_t bottomcursor;
uint16_t LRTimeoutmS;

//#define uS_TO_S_FACTOR 1000000ULL  //conversion factor for micro seconds to seconds

bool SDSAVEOK = false;     //set true if image save to SD OK
bool SDFOUND = false;      //set true if SD OK on intialisation
bool CAMERAFOUND = false;  //set to true if Camera initially detected OK
//bool READYACK = false;
bool IMAGEOK = false;



void loop() {
  OKLEDon();
  //READYACK = false;
  Monitorport.println();
  Monitorport.print(F("Waiting ready "));
  Monitorport.println(var.ListenCameraNumber);
  clear_TFT_Bottom_line();
  disp.setCursor(0, get_bottom_cursor(drotation));
  disp.print(F("Waiting ready "));
  disp.print(var.ListenCameraNumber);

  listen_packetLR(60000);  //listen for long range packets, ready signals, for 60secs

  LoRa.setupLoRa(var.Frequency, var.FreqOffset, var.SpreadingFactor, var.Bandwidth, var.CodeRate, var.Optimisation);  //switch to transfer mode
  clear_TFT_Bottom_line();
  disp.setCursor(0, get_bottom_cursor(drotation));
  disp.print("Receiving image");
  Monitorport.println("Receiving image");
  receive_image();

  if ((var.RequestedImage > 0) && (var.ListenCameraNumber > 0)) {
    disp.setCursor(0, get_bottom_cursor(drotation));
    disp.print("Requested image ");
    disp.print(var.RequestedImage);
    Monitorport.print("Waiting requested image ");
    Monitorport.println(var.RequestedImage);
    receive_image();

    if (IMAGEOK) {
      var.RequestedImage = 0;
    }

  } else {
    Monitorport.print("Requested image ");
    Monitorport.println(var.RequestedImage);
  }
}


void send_SENDACK1() {
  LoRa.startWriteSXBuffer(0);    //start the write at SX12XX internal buffer location 0
  LoRa.writeUint8(ATReadyACK1);  //packet type
  LoRa.writeUint8(var.ListenCameraNumber);
  LoRa.writeUint8(var.TXActions);
  TXPayloadL = LoRa.endWriteSXBuffer();  //closes packet write and returns the length of the payload to send

#ifdef DEBUG
  Monitorport.print(F("Transmit SENDACK1, packet length "));
  Monitorport.println(TXPayloadL + 4);
#endif

  TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, var.NetworkID, 10000, var.TXpower, WAIT_TX);
}


void send_SENDACK2() {
  LoRa.startWriteSXBuffer(0);    //start the write at SX12XX internal buffer location 0
  LoRa.writeUint8(ATReadyACK2);  //packet type
  LoRa.writeUint8(var.ListenCameraNumber);
  LoRa.writeUint8(var.TXActions);
  LoRa.writeUint32(var.Frequency);
  LoRa.writeUint8(var.SpreadingFactor);
  LoRa.writeUint8(var.Bandwidth);
  LoRa.writeUint8(var.CodeRate);
  TXPayloadL = LoRa.endWriteSXBuffer();  //closes packet write and returns the length of the payload to send

#ifdef DEBUG
  Monitorport.print(F("Transmit SENDACK2, packet length "));
  Monitorport.println(TXPayloadL + 4);
#endif

  TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, var.NetworkID, 10000, var.TXpower, WAIT_TX);
}


void send_SENDACK3() {
  uint8_t index1;
  LoRa.startWriteSXBuffer(0);    //start the write at SX12XX internal buffer location 0
  LoRa.writeUint8(ATReadyACK3);  //packet type
  LoRa.writeUint8(var.ListenCameraNumber);
  LoRa.writeUint8(var.TXActions);
  LoRa.writeUint32(var.Frequency);
  LoRa.writeUint8(var.SpreadingFactor);
  LoRa.writeUint8(var.Bandwidth);
  LoRa.writeUint8(var.CodeRate);
  LoRa.writeInt8(var.TXpower);
  LoRa.writeUint8(var.StandardImageSize);
  LoRa.writeUint8(var.SentImageSize);
  LoRa.writeUint8(var.SegmentSize);
  LoRa.writeUint8(var.PicturesToTake);
  LoRa.writeUint32(var.TimeToSleep);
  LoRa.writeUint32(var.RequestedImage);
  TXPayloadL = LoRa.endWriteSXBuffer();  //closes packet write and returns the length of the payload to send

#ifdef DEBUG
  Monitorport.print(F("Transmit SENDACK3, packet length "));
  Monitorport.println(TXPayloadL + 4);
#endif

  TXPacketL = LoRa.transmitSXReliable(0, TXPayloadL, var.NetworkID, 10000, var.TXpower, WAIT_TX);
}


bool receive_image() {
  uint32_t arraylength;
  int32_t num;
  IMAGEOK = false;
  OKLEDon();

  LoRa.setupLoRa(var.Frequency, var.FreqOffset, var.SpreadingFactor, var.Bandwidth, var.CodeRate, var.Optimisation);
  arraylength = ATreceiveArray(PSRAMptr, allocated_PSRAM, var.ATRXtimeoutmS);

  if (arraylength) {
    IMAGEOK = true;
    print_ImageSize(ATreturn_ATtype());
    Monitorport.println(F(" received"));
    Monitorport.print(F("Packets received "));
    Monitorport.println(ATreturn_RXOK());
    Monitorport.print(F("Packets errors "));
    Monitorport.println(ATreturn_ATRXErrors());
    Monitorport.print(F("Average SNR "));
    Monitorport.println(ATreturn_SNRtotal() / ATreturn_RXOK());

    clear_TFT_Bottom_line();
    disp.setCursor(0, bottomcursor);

    if (SDFOUND) {
      if (writeFile(ATFilenamebuff, PSRAMptr, arraylength))  //save picture to file
      {
        SDSAVEOK = true;
      } else {
        disp.print("SD Card Failed");
        ERRORSDon();
        SDSAVEOK = false;
        delay(1000);
      }
    } else {
      ERRORSDon();
      Monitorport.println(F("ERROR - No SD card"));
      disp.print("SD Card Failed");
      SDSAVEOK = false;
      delay(1000);
    }

    disp.fillScreen(TFT_BLACK);

    jpg.openRAM(PSRAMptr, arraylength, 0);
    jpgwidth = jpg.getWidth();
    jpgheight = jpg.getHeight();

    Monitorport.print("Read jpg Width ");
    Monitorport.println(jpgwidth);
    Monitorport.print("Read jpg Height ");
    Monitorport.println(jpgheight);

    if (jpgwidth > (dwidth * 4)) {
      scale = 8;
    } else if (jpgwidth > (dwidth * 2)) {
      scale = 4;
    } else if (jpgwidth > (dwidth)) {
      scale = 2;
    } else if (jpgwidth <= dwidth) {
      scale = 0;
    }

    Monitorport.print("Display Scale is ");
    Monitorport.println(scale);
    xcord = getCordinate(dwidth, jpgwidth, scale, drotation);
    ycord = getCordinate(dheight, jpgheight, scale, drotation);
    jd.loadJPEG(&disp, JPEGDISPLAY_CENTER, JPEGDISPLAY_CENTER, PSRAMptr, arraylength, scale);

    disp.setCursor(0, 0);
    disp.print(ATFilenamebuff);
    disp.print(" ");
    disp.print(jpgwidth);
    disp.print("x");
    disp.print(jpgheight);

    ATFilenamebuff[0] = '0';  //needed to make atoi work
    num = atoi(ATFilenamebuff);
    return true;
  } else {
    Monitorport.println(F("No Image received"));
    return false;
  }
  return true;
}


uint16_t get_bottom_cursor(uint16_t temprotate) {
  if ((temprotate == 90) || (temprotate == 270)) {
    return 224;
  } else {
    return 309;
  }
}


uint16_t getCordinate(uint16_t tftdimension, uint16_t imagedimension, uint8_t scale, uint16_t therotation) {
  int16_t cordinate, todisplaydimension, centrecord;

  if (scale == 0) {
    scale = 1;
  }

  todisplaydimension = imagedimension / scale;
  centrecord = (tftdimension / 2) - 1;

  if (imagedimension > tftdimension) {
    cordinate = centrecord - (imagedimension / (scale * 2));
  } else {
    cordinate = centrecord - ((imagedimension / scale) / 2);
  }

#ifdef DEBUG
  Monitorport.print("tftdimension ");
  Monitorport.println(tftdimension);
  Monitorport.print("imagedimension ");
  Monitorport.println(imagedimension);
  Monitorport.print("scale ");
  Monitorport.println(scale);
  Monitorport.print("todisplaydimension ");
  Monitorport.println(todisplaydimension);
  Monitorport.print("centrecord ");
  Monitorport.println(centrecord);
  Monitorport.print("Calculated cordinate ");
  Monitorport.println(cordinate);
#endif

  if (cordinate < 0) {
    cordinate = 0;
  }

#ifdef DEBUG
  Monitorport.print("returned cordinate ");
  Monitorport.println(cordinate);
#endif
  return cordinate;
}


bool writeFile(const char *path, uint8_t *data, size_t len) {
  OKLEDon();
  Monitorport.printf("Writing file to SD: %s\r\n", path);

  File file = SD_MMC.open(path, FILE_WRITE);

  if (!file) {
    Monitorport.println("ERROR - Failed to open file for writing");
    delay(1000);
    return false;
  }

  if (file.write(data, len) == len) {
    Monitorport.println("File written");
    file.close();
    Monitorport.println(F("File close"));
    return true;
  } else {
    Monitorport.println("ERROR - Write failed");
    clear_TFT_Bottom_line();
    disp.setCursor(0, bottomcursor);
    disp.print("SD Card Failed");
    ERRORSDon();
    delay(1000);
    return false;
  }
}


void listen_packetLR(uint32_t waitmS) {
  uint8_t readchar;
  uint32_t startwaitmS, temp;
  Monitorport.println("Listening for ready broadcast");
  LoRa.setupLoRa(var.LRFREQ, var.LRFREQOFF, var.LRSF, var.LRBW, var.LRCR, var.LROPT);  //configure long range var.Frequency and LoRa settings
  startwaitmS = millis();

  LoRa.receiveSXReliable(0, var.NetworkID, 0, NO_WAIT);
  LRTimeoutmS = millis();

  do {
    if (Monitorport.available() > 0) {
      readchar = Monitorport.read();
      menu_Main(var.MenuTimeout);
      LoRa.receiveSXReliable(0, var.NetworkID, 0, NO_WAIT);
      Monitorport.println("Listening for ready broadcast");
      startwaitmS = millis();  //restart timeout timer
      continue;
    }

    if (digitalRead(LORA_DIO1)) {
      LoRa.setMode(MODE_STDBY_RC);  //ensure to stop further packet reception
      startwaitmS = millis();       //restart timeout timer
      RXCameraNumber = LoRa.getByteSXBuffer(1);

      Monitorport.print(F("Packet received from "));
      Monitorport.println(RXCameraNumber);
      clear_TFT_Bottom_line();
      disp.setCursor(0, get_bottom_cursor(drotation));
      disp.print(F("Packet from "));
      disp.print(RXCameraNumber);

      if ((var.ListenCameraNumber != 0) && (var.ListenCameraNumber != RXCameraNumber)) {
        Monitorport.println(F("No Camera match"));
        Monitorport.print(F("ListenCameraNumber "));
        Monitorport.println(var.ListenCameraNumber);
        Monitorport.print(F("RXCameraNumber "));
        Monitorport.println(RXCameraNumber);
        LoRa.receiveSXReliable(0, var.NetworkID, 0, NO_WAIT);
        Monitorport.println();
        Monitorport.println("Listening for ready broadcast");
        continue;
      }

      Monitorport.println(F("Camera match"));
      RXPacketL = LoRa.readRXPacketL();
      RXPacketType = LoRa.getByteSXBuffer(0);  //read single byte from start of buffer
      process_Packet(RXPacketType);
      return;
    }
  } while (millis() < (startwaitmS + waitmS));

  temp = (millis() - LRTimeoutmS) / 1000;
  Monitorport.print(F("LR Listen Timeout "));
  Monitorport.print((millis() - LRTimeoutmS) / 1000);
  Monitorport.println(F("s"));
  clear_TFT_Bottom_line();
  disp.setCursor(0, get_bottom_cursor(drotation));
  disp.print(F("LR Timeout "));
  disp.print(temp);
  disp.print(F("s"));
  return;
}


void process_Packet(uint8_t type) {

  if (type == ATReady) {
    RXCameraNumber = LoRa.getByteSXBuffer(1);
    Monitorport.print(F("Ready from "));
    Monitorport.println(RXCameraNumber);
    clear_TFT_Bottom_line();
    disp.setCursor(0, get_bottom_cursor(drotation));
    disp.print(F("Ready from "));
    disp.print(RXCameraNumber);
    delay(250);

    var.TXFlags = LoRa.getByteSXBuffer(2);

    if (bitRead(var.TXFlags, TXFlashStoreValid)) {
      Monitorport.println(F("TX Using FLASH Store settings"));
    } else {
      Monitorport.println(F("TX Using program defaults"));
    }

    if (SENDACK3 || var.EnforceExtendedSettings) {
      if (var.EnforceExtendedSettings) {
        Monitorport.println(F("Enforce Extended Settings"));
      }
      SENDACK3 = false;
      SENDACK2 = false;
      Monitorport.println(F("Send ATReadyACK3 settings"));
      send_SENDACK3();
      bitClear(var.TXActions, TXerasestore);  //clear any request to erase store
      bitClear(var.TXActions, TXsavestore);   //clear any request to save settings to store
      return;
    }

    if (SENDACK2 || var.EnforceBasicSettings) {
      if (var.EnforceBasicSettings) {
        Monitorport.println(F("Enforce Basic Settings"));
      }
      SENDACK3 = false;
      SENDACK2 = false;
      Monitorport.println(F("Send ATReadyACK2 settings"));
      send_SENDACK2();
      bitClear(var.TXActions, TXerasestore);  //clear request to save settings to store
      bitClear(var.TXActions, TXsavestore);   //clear request to save settings to store
      return;
    }

    //use settings from 'broadcast_Ready_Settings()' packet sent by transmitter and send an ack
    Monitorport.println(F("Use settings from TX ready broadcast"));
    process_ReadySettings();
    send_SENDACK1();
    bitClear(var.TXActions, TXerasestore);  //clear request to save settings to store
    bitClear(var.TXActions, TXsavestore);   //clear request to save settings to store
    return;
  }
  return;
}


void process_ReadySettings() {
  LoRa.startReadSXBuffer(3);
  var.Frequency = LoRa.readUint32();
  var.SpreadingFactor = LoRa.readUint8();
  var.Bandwidth = LoRa.readUint8();
  var.CodeRate = LoRa.readUint8();
  LoRa.endReadSXBuffer();
  Monitorport.print(F("Received settings for "));
  Monitorport.println(RXCameraNumber);
  Monitorport.println();
  print_active_settings();
  return;
}


bool setupLoRaDevice() {

  if (!LoRa.begin(LORA_NSS, LORA_RESET, LORA_BUSY, LORA_DIO1, LORA_DEVICE)) {
    return false;
  }

  if ((LORA_DEVICE == DEVICE_SX1261) || (LORA_DEVICE == DEVICE_SX1262) || (LORA_DEVICE == DEVICE_SX1262)) {
    usingSX126X = true;
    usingSX127X = false;
  } else {
    usingSX126X = false;
    usingSX127X = true;
  }
  return true;
}


bool setupMMCSD() {

  SD_MMC.setPins(MMC_SCK, MMC_CMD, MMC_D0);

  if (!SD_MMC.begin("/sdcard", true))  //start MMC in 1 bit mode
  {
    return false;
  } else {
    uint8_t cardType = SD_MMC.cardType();
    if (cardType == CARD_NONE) {
      Monitorport.println("ERROR - SD MMC card not recognised");
      return false;
    } else {
      return true;
    }
  }
  return true;
}


bool check_PSRAM() {

  if (psramInit()) {
    available_PSRAM = ESP.getFreePsram();
    Monitorport.print("PSRAM available: ");
    Monitorport.print(available_PSRAM);
    Monitorport.println(" bytes");
    allocated_PSRAM = available_PSRAM / 2;  //dont use all PSRAM for array
    Monitorport.print(allocated_PSRAM);
    Monitorport.println(" bytes of PSRAM for image");
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


void init_TFT_Display() {
  pinMode(DISP_LED, OUTPUT);
  digitalWrite(DISP_LED, HIGH);  //display screen on
  disp.begin(LCD_ILI9341, FLAGS_NONE, &disp_SPI, DISP_CS, DISP_DC, DISP_RST, -1);
  disp.fillScreen(TFT_BLACK);    // the default is to not keep a local copy of the framebuffer since it's large
  disp.setTextColor(TFT_GREEN);  // all functions will send data to the display and be visible immediately
  disp.setFont(FONT_12x16);      // Choice of fonts is FONT_8x8, FONT_12x16, FONT_16x16, FONT_16x32
  disp.setRotation(drotation);
  disp.setCursor(0, 13);
  disp.println("LoRa Image Receiver");

  if ((drotation == 90) || (drotation == 270)) {
    dwidth = tftwidth;
    dheight = tftheight;
  } else {
    dwidth = tftheight;
    dheight = tftwidth;
  }
}


void print_GPIO() {
  Monitorport.print("DISP_SCK ");
  Monitorport.println(DISP_SCK);
  Monitorport.print("DISP_MISO ");
  Monitorport.println(DISP_MISO);
  Monitorport.print("DISP_MOSI ");
  Monitorport.println(DISP_MOSI);
  Monitorport.print("DISP_CS ");
  Monitorport.println(DISP_CS);
  Monitorport.print("DISP_DC ");
  Monitorport.println(DISP_DC);
  Monitorport.print("DISP_RST ");
  Monitorport.println(DISP_RST);
  Monitorport.print("SD_CS ");
  Monitorport.println(SD_CS);
  Monitorport.print("MMC_SCK ");
  Monitorport.println(MMC_SCK);
  Monitorport.print("MMC_CMD ");
  Monitorport.println(MMC_CMD);
  Monitorport.print("MMC_D0 ");
  Monitorport.println(MMC_D0);
  Monitorport.print("LORA_SCK ");
  Monitorport.println(LORA_SCK);
  Monitorport.print("LORA_MISO ");
  Monitorport.println(LORA_MISO);
  Monitorport.print("LORA_MOSI ");
  Monitorport.println(LORA_MOSI);
  Monitorport.print("LORA_NSS ");
  Monitorport.println(LORA_NSS);
  Monitorport.print("LORA_RESET ");
  Monitorport.println(LORA_RESET);
  Monitorport.print("LORA_BUSY ");
  Monitorport.println(LORA_BUSY);
  Monitorport.print("LORA_DIO1 ");
  Monitorport.println(LORA_DIO1);
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;
  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delaymS);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delaymS);
  }
}


void clear_TFT_Bottom_line() {
  disp.setCursor(0, bottomcursor);
  disp.print(F("                          "));
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


void rgboff() {
  rgbLedWrite(RGB_LED, 0, 0, 0);  //ledoff
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  led_Flash(4, 125);  //flash LED to indicate program start

  OKLEDon();
  pinMode(DISP_RST, OUTPUT);
  digitalWrite(DISP_RST, HIGH);
  pinMode(DISP_CS, OUTPUT);
  digitalWrite(DISP_CS, HIGH);
  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);
  pinMode(LORA_NSS, OUTPUT);
  digitalWrite(LORA_NSS, HIGH);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  digitalWrite(LORA_NSS, HIGH);
  pinMode(LORA_NSS, OUTPUT);  //disable LoRa device for now

  Monitorport.begin(115200);
  Monitorport.println();
  Monitorport.println(__FILE__);
  Monitorport.println();

#ifdef DEBUG
  print_GPIO();
#endif

  init_store();

  if (check_store())  //check variables in Flash store, if structure valid use them
  {
    Monitorport.println();
    Monitorport.println(F("**** FLASH Store valid ****"));
    copy_store_var();
    Monitorport.println(F("Using FLASH Store settings"));
    Monitorport.println();

  } else {
    Monitorport.println();
    Monitorport.println(F("**** FLASH Store not valid ****"));
    Monitorport.println(F("Using program defaults"));
  }

  disp_SPI.begin(DISP_SCK, DISP_MISO, DISP_MOSI, -1);  // don't let it control the CS line of either device
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);

  init_TFT_Display();

  bottomcursor = get_bottom_cursor(drotation);

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

  clear_TFT_Bottom_line();
  disp.setCursor(0, bottomcursor);

  if (setupLoRaDevice()) {
    Monitorport.println(F("LoRa device OK"));
    disp.print("LoRa device OK");
  } else {
    Monitorport.println("ERROR - LoRa Device Failed");
    Monitorport.println("Program halted");
    disp.print("LoRa Device Failed");
    ERRORon();
    while (1)
      ;
  }

  clear_TFT_Bottom_line();
  disp.setCursor(0, bottomcursor);

  if (setupMMCSD()) {
    Monitorport.println("SD MMC Card Mount OK");
    disp.print("SD Card OK");
    SDFOUND = true;
  } else {
    Monitorport.println("ERROR - SD MMC Card Mount Failed");
    disp.print("SD Card Failed");
    SDFOUND = false;
    ERRORSDon();
    delay(2000);
  }

  var.TXCameraNumber = 0;
  var.ListenCameraNumber = 0;
  pinMode(SW13, INPUT_PULLUP);
  Monitorport.println();
  print_active_settings();
}
