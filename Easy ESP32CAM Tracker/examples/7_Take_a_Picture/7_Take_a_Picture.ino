/***********************************************************************************************
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-ov2640-camera-settings/
***********************************************************************************************/


/***********************************************************************************************
  Modified by Stuart Robinson 16/12/20
***********************************************************************************************/

/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a demonstration of using the ESP32CAM to take pictures and save to SD card.
  It is intended to be used on the ESP32CAM board that has an SPI LoRa module shareing the SD card pins.

  Wakes up, takes a picture, saves it to SD card and goes back to sleep.

  Serial monitor baud rate is set at 115200
*******************************************************************************************************/

#include <Arduino.h>
#include "soc/soc.h"                             //disable brownout problems
#include "soc/rtc_cntl_reg.h"                    //disable brownout problems
#include "driver/rtc_io.h"

#include "CameraCode.h"

const uint8_t REDLED = 33;                       //pin number for ESP32CAM on board red LED, set logic level low for on
const uint8_t WHITELED = 4;                      //pin number for ESP32CAM on board white LED, set logic level high for on
const uint8_t NSS = 13;                          //NSS (selectpin) used to disable LoRa module

const uint16_t SleepTimesecs = 10;               //sleep time in seconds after each TX loop
const uint32_t uS_TO_S_FACTOR = 1000000;         //Conversion factor for micro seconds to seconds
const uint8_t PicturesToTake = 3;                //number of pictures to take at each wakeup
const uint32_t PictureDelaymS = 1000;            //delay in mS between pictures

RTC_DATA_ATTR int16_t bootCount = 0;             //variables to save in RTC ram
RTC_DATA_ATTR uint16_t sleepcount = 0;


void loop()
{
  pinMode(NSS, OUTPUT);
  digitalWrite(NSS, HIGH);                         //start with LoRa module disabled

  Serial.println();
  Serial.println(F("Awake !"));
  Serial.print(F("Bootcount "));
  Serial.println(bootCount);
  Serial.print(F("Sleepcount "));
  Serial.println(sleepcount);

  configInitCamera();
  initMicroSDCard();

  takeSavePhoto(PicturesToTake, PictureDelaymS);

  pinMode(WHITELED, OUTPUT);
  digitalWrite(WHITELED, LOW);                   //turns off the ESP32-CAM white LED connected to GPIO 4
  rtc_gpio_hold_en(GPIO_NUM_4);                  //so LED stays off in sleep

  esp_sleep_enable_timer_wakeup(15 * uS_TO_S_FACTOR);
  Serial.print(F("Start Sleep "));
  Serial.print(SleepTimesecs);
  Serial.println(F("s"));
  Serial.flush();

  sleepcount++;
  esp_deep_sleep_start();
  Serial.println("This should never be printed !!!");
}


void redFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(REDLED, OUTPUT);                        //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, LOW);
    delay(ondelaymS);
    digitalWrite(REDLED, HIGH);
    delay(offdelaymS);
  }
  pinMode(REDLED, INPUT);                        //setup pin as input
}


void whiteFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(WHITELED, OUTPUT);                     //setup pin as output

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(WHITELED, HIGH);
    delay(ondelaymS);
    digitalWrite(WHITELED, LOW);
    delay(offdelaymS);
  }
}


void setup()
{
  redFlash(5, 100, 100);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  rtc_gpio_hold_dis(GPIO_NUM_4);             //white LED pin back to normal control after sleep

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("7_Take_a_Picture starting");

  if (bootCount == 0)                        //run this only the first time after programming or power up
  {
    bootCount = bootCount + 1;
  }
}


