/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/08/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a program that blinks the 3 x WS2811 Neopixel LEDs on the NodeMCU32 shield
  red, green, blue and white.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/


#include <Adafruit_NeoPixel.h>                  //get library here > https://github.com/adafruit/Adafruit_NeoPixel

#define NPIXEL 33                               //WS2811 neopixels on this pin
#define NUMPIXELS 3                             //How many NeoPixels are attached
#define BRIGHTNESS 50                           //LED brightness 0 to 255, full brightness 255 approx 11mA per colour 
#define PERIOD 125                              //time in mS for LED to be on

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NPIXEL, NEO_GRB + NEO_KHZ800); //get library here > https://github.com/adafruit/Adafruit_NeoPixel


void loop()
{
  Neo_FlashRed(1, PERIOD, 0, BRIGHTNESS);         //first neopixel in chain
  Neo_FlashBlue(1, PERIOD, 0, BRIGHTNESS);
  Neo_FlashGreen(1, PERIOD, 0, BRIGHTNESS);
  Neo_FlashWhite(1, PERIOD, 0, BRIGHTNESS);
  
  Neo_FlashRed(1, PERIOD, 1, BRIGHTNESS);         //second neopixel in chain
  Neo_FlashBlue(1, PERIOD, 1, BRIGHTNESS);
  Neo_FlashGreen(1, PERIOD, 1, BRIGHTNESS);
  Neo_FlashWhite(1, PERIOD, 2, BRIGHTNESS);
  
  Neo_FlashRed(1, PERIOD, 2, BRIGHTNESS);         //third neopixel in chain
  Neo_FlashBlue(1, PERIOD, 2, BRIGHTNESS);
  Neo_FlashGreen(1, PERIOD, 2, BRIGHTNESS);
  Neo_FlashWhite(1, PERIOD, 2, BRIGHTNESS);
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
    pixels.setPixelColor(number, pixels.Color(brightness,brightness,brightness)); //White on
    pixels.show();
    delay(delaymS);
    pixels.setPixelColor(number, pixels.Color(0, 0, 0));          //all colours off
    pixels.show();
    delay(delaymS);
  }
}



void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));                 //all colours off
  pixels.show();
}
