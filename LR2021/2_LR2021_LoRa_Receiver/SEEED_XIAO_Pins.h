/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//for LR2021 on SEEED XIAO SAMD and nrf52840 plus

#define LORA_NSS D3    //select pin on LoRa device
#define LORA_RESET D0  //reset pin on LoRa device
#define LORA_BUSY D2   //busy pin on LoRa device
#define LORA_DIO D1    //DIO pin on LoRa device, used for RX and TX done

#define DIOBUZZER 10  //LR2021 DIO pin that has buzzer connected
#define ENABLEBUZZER  //comment out to disable buzzer on LR2021

#define LED1 13      //built in LED on XIAO SAMD21
#define LEDON LOW    //for built in LED on XIAO SAMD21
#define LEDOFF HIGH  //for built in LED on XIAO SAMD21

#define SWITCH0 D5    //SCL pin
#define SWITCH1 D4    //SDA pin

//Serial port definition
#define mySerial Serial  //use Serial to send Serial output to USB connector, to use the XIAO Serial port pins on D6 and D7
