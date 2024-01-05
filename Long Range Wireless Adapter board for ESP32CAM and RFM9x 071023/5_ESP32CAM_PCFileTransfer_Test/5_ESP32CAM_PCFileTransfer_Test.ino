/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/02/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a test program to allow you to check if the YModem link to a PC is setup 
  and working correctly.

  The program creates an array in PSRAM and then fills it with 8 'lines' of ASCII chatacters from 0x20 to
  0x7E. If Tera Term on the PC is setup correctly the resulting file should appear in the selected PC 
  folder, it is creates as a text file and can be viewed in Notepad. The lines of the transferred file
  should look like this;
  
   !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~ 

  Progress and monitor messages are set to the Serial2 TX pin which is set at pin 33, the Red LED pin. 
  A wire link needs to be added to connect the Serial2 output connector, CONA on the ESP32CAM Long Range
  Wireless Adapter PCB.
  
  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#include <SPI.h>


#define REDLED 33                          //pin number for ESP32CAM on board red LED, set logic level low for on
#define RXD2 -1                            //RX pin for Serial2 monitor port, not used 
#define TXD2 33                            //TX pin for Serial2 monitor port

const uint32_t ARDTarraysize = 0x20000;    //maximum file\array size to receive


uint8_t *PSRAMptr;                         //create a global pointer to the array to send, so all functions have access
char ARDTfilenamebuff[] = "TestFile.txt";
uint32_t arraylength;
uint32_t bytestransfered;

#include "YModemArray.h"                   //YModem transferring array functions, local file


void loop()
{
  Serial2.println(F("YModem file transfer starting"));
  Serial2.flush();
  led_Flash(1,1000);
  bytestransfered = yModemSend(ARDTfilenamebuff, PSRAMptr, arraylength, 1, 1);
  Serial2.flush();
  led_Flash(1,1000);

  if (bytestransfered == arraylength)
  {
  Serial2.println(F("YModem transfer OK"));
  }
  else
  {
  Serial2.println(F("YModem transfer FAILED"));
  }
  Serial2.println();
  Serial2.flush();
  delay(10000);
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;
  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(REDLED, HIGH);
    delay(delaymS);
    digitalWrite(REDLED, LOW);
    delay(delaymS);
  }
}


void setup()
{
  uint32_t available_PSRAM_size;
  uint32_t new_available_PSRAM_size;
  uint8_t index1, index2;

  pinMode(REDLED, OUTPUT);                            //setup pin as output for indicator LED
  
  Serial.begin(115200);                               //Ymodem transfer port

  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);      //debug port, format is Monitorport.begin(baud-rate, protocol, RX pin, TX pin)
  Serial2.println();
  Serial2.println(__FILE__);
  Serial2.println();
  Serial2.println();

  if (psramInit())
  {
    Serial2.println("PSRAM is correctly initialised");
    available_PSRAM_size = ESP.getFreePsram();
    Serial2.println((String)"PSRAM Size available: " + available_PSRAM_size);
  }
  else
  {
    Serial2.println("PSRAM not available");
    while (1);
  }

  Serial2.println("Allocate array in PSRAM");
  uint8_t *byte_array = (uint8_t *) ps_malloc(ARDTarraysize * sizeof(uint8_t));
  PSRAMptr = byte_array;                              //save the pointe to byte_array to global pointer

  new_available_PSRAM_size = ESP.getFreePsram();
  Serial2.print("PSRAM Size available: ");
  Serial2.println(new_available_PSRAM_size);
  Serial2.print("PSRAM array bytes allocated: ");
  Serial2.println(available_PSRAM_size - new_available_PSRAM_size);

  //fill array with test data
  arraylength =0;

  for (index1 = 0; index1 <= 8; index1++)
  {
   for (index2 = 0x20; index2 <= 0x7E; index2++)
   {
   PSRAMptr[arraylength] = index2;
   arraylength++;
   }
   PSRAMptr[arraylength++] = 0x0d;
   PSRAMptr[arraylength++] = 0x0a;
  }

  Serial2.print("Array bytes written ");
  Serial2.println(arraylength);
  Serial2.println();
}
