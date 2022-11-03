/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 03/11/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO RP2040.

  Program Operation - This is a program that blinks the WS2812 Neopixel LED on the XIAO RP2040 board.
  Needs the latest Adafruit Neopixel library as of October 2022.

  Uses the earlephilhower/arduino-pico: Raspberry Pi Pico Arduino core, for all RP2040 boards.
  Get it here > https://github.com/earlephilhower/arduino-pico
  Board selected is 'Seeed XIAO RP2040'

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/


#include <Adafruit_NeoPixel.h>                  //get library here > https://github.com/adafruit/Adafruit_NeoPixel

#define NEOPIX 12                               //Neopixel on this pin
#define NEO_PWR 11                              //Neopixel power
#define NUMPIXELS 1                             //How many NeoPixels are attached
#define BRIGHTNESS 50                           //LED brightness 0 to 255 

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIX, NEO_GRB + NEO_KHZ800); //get library here > https://github.com/adafruit/Adafruit_NeoPixel


void loop()
{
  Serial.println(F("Green"));
  Neo_FlashGreen(4, 125, 0, BRIGHTNESS);
  Serial.println(F("Red"));
  Neo_FlashRed(4, 125, 0, BRIGHTNESS);
  Serial.println(F("Blue"));
  Neo_FlashBlue(4, 125, 0, BRIGHTNESS);
  Serial.println(F("White"));
  Neo_FlashWhite(4, 125, 0, BRIGHTNESS);

  delay(2000);
}


void Neo_FlashGreen(uint16_t flashes, uint16_t delaymS, uint8_t number, uint8_t brightness)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    pixels.setPixelColor(number, pixels.Color(brightness, 0, 0)); //Green on
    pixels.show();
    delay(delaymS);
    pixels.setPixelColor(number, pixels.Color(0, 0, 0));          //all colours off
    pixels.show();
    delay(delaymS);
  }
}


void Neo_FlashRed(uint16_t flashes, uint16_t delaymS, uint8_t number, uint8_t brightness)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    pixels.setPixelColor(number, pixels.Color(0, brightness, 0)); //Red on
    pixels.show();
    delay(delaymS);
    pixels.setPixelColor(number, pixels.Color(0, 0, 0));          //all colours off
    pixels.show();
    delay(delaymS);
  }
}


void Neo_FlashBlue(uint16_t flashes, uint16_t delaymS, uint8_t number, uint8_t brightness)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    pixels.setPixelColor(number, pixels.Color(0, 0, brightness)); //Blue on
    pixels.show();
    delay(delaymS);
    pixels.setPixelColor(number, pixels.Color(0, 0, 0));          //all colours off
    pixels.show();
    delay(delaymS);
  }
}


void Neo_FlashWhite(uint16_t flashes, uint16_t delaymS, uint8_t number, uint8_t brightness)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    pixels.setPixelColor(number, pixels.Color(brightness, brightness, brightness)); //White on
    pixels.show();
    delay(delaymS);
    pixels.setPixelColor(number, pixels.Color(0, 0, 0));          //all colours off
    pixels.show();
    delay(delaymS);
  }
}



void setup()
{
  pinMode(NEO_PWR, OUTPUT);
  digitalWrite(NEO_PWR, HIGH);                                     //power to neopixel

  Serial.begin(115200);
  Serial.println(F("23_Neopixel_Blink Starting"));
  Serial.println();

  pixels.begin();                                                 //this initializes the NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));                 //all colours off
  pixels.show();
}
