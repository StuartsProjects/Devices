/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 19/4/26

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a LED blink program for the RGB Neopixel LED on the ESP32S3 Camera
  Development Board. In the Arduino IDE set the board type to be 'ESP32S3 DEV Module'.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/


#define LED1 RGB_BUILTIN  //GPIO 97
#define RGB_LED 48        //pin number for neopixel LED
#define BRIGHTNESS 64     //default brightness (max 255)


void loop() {
  Serial.print(millis() / 1000);
  Serial.println(F(" Seconds"));

  Serial.println(F("RGB - Red"));
  rgb_Flash(1, 100, BRIGHTNESS, 0, 0);
  delay(900);

  Serial.println(F("RGB - Green"));
  rgb_Flash(1, 100, 0, BRIGHTNESS, 0);
  delay(900);

  Serial.println(F("RGB - Blue"));
  rgb_Flash(1, 100, 0, 0, BRIGHTNESS);
  delay(900);

  Serial.println(F("RGB - White"));
  rgb_Flash(1, 100, BRIGHTNESS, BRIGHTNESS, BRIGHTNESS);
  delay(900);
}


void rgb_Flash(uint16_t flashes, uint16_t delaymS, uint8_t Redbright, uint8_t Greenbright, uint8_t Bluebright) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    rgbLedWrite(RGB_LED, Redbright, Greenbright, Bluebright);  //RGB LED on
    delay(delaymS);
    rgbLedWrite(RGB_LED, 0, 0, 0);  //RGB LED off
    delay(delaymS);
  }
}


void led_Flash(uint16_t flashes, uint16_t delaymS) {
  uint16_t index;

  for (index = 1; index <= flashes; index++) {
    digitalWrite(LED1, HIGH);  //LED on
    delay(delaymS);
    digitalWrite(LED1, LOW);  //LED off
    delay(delaymS);
  }
}


void setup() {
  pinMode(LED1, OUTPUT);
  led_Flash(5, 250);  //flash Neopixel as LED

  Serial.println();
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println(F("Program starting"));
  Serial.print(F("RGB_BUILTIN on pin "));
  Serial.println(RGB_BUILTIN);
  Serial.println();
  delay(1000);
}
