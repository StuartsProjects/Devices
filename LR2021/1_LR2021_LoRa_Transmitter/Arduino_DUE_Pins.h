/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 16/10/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

//for LR2021 on Arduino DUE Shield

#define LORA_NSS 10   //select pin on LoRa device
#define LORA_RESET 9  //reset pin on LoRa device
#define LORA_BUSY 7   //busy pin on LoRa device
#define LORA_DIO 3    //DIO pin on LoRa device, used for RX and TX done

#define DIOBUZZER 10  //LR2021 DIO pin that has buzzer connected
#define ENABLEBUZZER  //comment out to disable buzzer on LR2021

#define LED1 8      //for Arduino_DUE
#define LEDON HIGH  //for Arduino_DUE
#define LEDOFF LOW  //for Arduino_DUE

#define SWITCH0 52  //CS pin on MB2 DUE Shield 200420
#define SWITCH1 33  //RST pin on MB2 DUE Shield 200420
