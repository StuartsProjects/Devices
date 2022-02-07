/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 29/01/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - The program flashes the ESP32CAM red LED in pin 33 4 times and then puts the ESP32
  into a deep_sleep for a period determined by the TIME_TO_SLEEP variable (in seconds). On wakeup the red
  LED flshes again. It is assumed a LoRa module is connected to the pins shown. If the LoRa device is not
  found the redled will flash continuously.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>
#include <SX127XLT.h>                            //include the appropriate library  
#include <ProgramLT_Definitions.h>
SX127XLT LoRa;                                   //create a library class instance called LoRa

#include <Arduino.h>
#include "soc/soc.h"                             //disable brownout problems
#include "soc/rtc_cntl_reg.h"                    //disable brownout problems
#include "driver/rtc_io.h"

//updated pinouts for 'ESP32CAM_Long_Range_Wireless_Adapter PCB dated 261121
//Note transistor driving the White LED on pin 4, or the White LED itself needs to be removed

#define NSS 12                //select on LoRa device
#define NRESET 15             //reset pin on LoRa device
#define SCK 4                 //SCK on SPI3
#define MISO 13               //MISO on SPI3 
#define MOSI 2                //MOSI on SPI3
#define REDLED 33             //pin number for ESP32CAM on board red LED, set logic level low for on

#define CAM_PWR 32
#define LORA_DEVICE DEVICE_SX1278                //this is the device we are using

#define uS_TO_S_FACTOR 1000000                   //Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  15                        //Time ESP32 will go to sleep (in seconds) 

RTC_DATA_ATTR int32_t bootCount = 0;
RTC_DATA_ATTR uint32_t sleepcount = 0;


void loop()
{
  Serial.print(F("Bootcount "));
  Serial.println(bootCount);
  Serial.print(F("Sleepcount "));
  Serial.println(sleepcount);
  Serial.println(F("LED Flash"));

  led_Flash(4, 125);
  delay(1000);

  LoRa.setSleep(CONFIGURATION_RETENTION);

  rtc_gpio_hold_en(GPIO_NUM_13);                 //hold LoRa device off in sleep

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println(F("Starting Sleep"));
  Serial.flush();
  sleepcount++;
  esp_deep_sleep_start();

  Serial.println();
  Serial.println(F("Awake !"));
}


void led_Flash(unsigned int flashes, unsigned int delaymS)
{
  //flash LED to show tracker is alive
  unsigned int index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, LOW);
    delay(delaymS);
    digitalWrite(REDLED, HIGH);
    delay(delaymS);
  }
}


void setup()
{
  rtc_gpio_hold_dis(GPIO_NUM_12);               //LoRa NSS back to normal control after sleep

  digitalWrite(REDLED, LOW);
  pinMode(REDLED, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.print(__FILE__);
  Serial.println();

  Serial.println(F("Program Starting"));

  if (bootCount == 0)                           //Run this only the first time
  {
    bootCount = bootCount + 1;
  }

  SPI.begin(SCK, MISO, MOSI, NSS);

  if (LoRa.begin(NSS, LORA_DEVICE))
  {
    Serial.println(F("LoRa device found"));
    Serial.println();
  }
  else
  {
    Serial.println(F("ERROR - No LoRa Device found"));
    Serial.println();
    while (1)
    {
      led_Flash(50, 100);                       //long fast speed LED flash indicates LoRa device error
    }
  }
}
