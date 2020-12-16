/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/12/20

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program is a basic test program for the ESP32CAM Tracker board. At startup it
  blinks the pin 33 LED then blinks the white LED.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#define REDLED 33                                  //pin number for ESP32CAM on board red LED, set logic level low for on
#define WHITELED 4                                 //pin number for ESP32CAM on board white LED, set logic level high for on


void loop()
{
  Serial.println(F("Red LED flash"));
  redFlash(5, 100, 100);
  Serial.println(F("White LED flash"));
  whiteFlash(5, 5, 195);
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
  pinMode(REDLED, INPUT);                            //setup pin as input
}


void whiteFlash(uint16_t flashes, uint16_t ondelaymS, uint16_t offdelaymS)
{
  uint16_t index;

  pinMode(WHITELED, OUTPUT);                          //setup pin as output

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
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println(F("Serial.begin(115200)"));
  Serial.println(F("1_LED_Blink for ESP32CAM Starting"));
}

