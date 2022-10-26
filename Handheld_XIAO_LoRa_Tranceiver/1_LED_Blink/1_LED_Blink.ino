/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  The XIAO pin 13 LED, is blinked. The blinks should be close to one per second. Messages are sent to the
  Serial Monitor also.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#define LED1 13                                //on board LED is yellow

uint16_t seconds;                              //used to display time elapsed on Serial Monitor


void loop()
{
  Serial.print(seconds);
  Serial.println(F(" Seconds"));               //this message should print on console at close to once per second
  seconds++;
  digitalWrite(LED1, LOW);
  delay(100);
  digitalWrite(LED1, HIGH);
  delay(890);                                  //should give approx 1 second flash
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);                    //board LED on
    delay(delaymS);
    digitalWrite(LED1, LOW);                     //board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                         //setup board LED pin as output
  digitalWrite(LED1, HIGH);
  led_Flash(10, 125);                            //10 quick LED flashes to indicate program start
  delay(2000);

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("1_LED_Blink Starting"));
}
