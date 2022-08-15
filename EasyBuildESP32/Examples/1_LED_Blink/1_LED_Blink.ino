/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/08/22

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program blinks an LED connected the pin number defined below. The pin 2 LED,
  fitted to the NodeMCU32 is blinked as well. The blinks should be close to one per second. messages are
  sent to the Serial Monitor also.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#define LED1 33                                //pin number for shield LED, set logic level high for on

uint16_t seconds;                              //used to display time elapsed on Serial Monitor


void loop()
{
  Serial.print(seconds);
  Serial.println(F(" Seconds"));               //this message should print on console at close to once per second
  seconds++;
  digitalWrite(LED1, HIGH);
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(LED1, LOW);
  digitalWrite(2, LOW);
  delay(890);                                  //should give approx 1 second flash
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);                  //LED on
    digitalWrite(2, HIGH);                     //Arduino board LED on
    delay(delaymS);
    digitalWrite(LED1, LOW);                   //LED off
    digitalWrite(2, LOW);                      //Arduino board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                       //setup pin as output for indicator LED
  pinMode(2, OUTPUT);                          //setup pin as output for some Arduino boards that include an LED on pin 13
  digitalWrite(LED1, HIGH);
  digitalWrite(2, HIGH);

  led_Flash(2, 125);                           //two quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.println();
  Serial.println(__TIME__);
  Serial.println();
  Serial.println(__DATE__);
  Serial.println();
}
