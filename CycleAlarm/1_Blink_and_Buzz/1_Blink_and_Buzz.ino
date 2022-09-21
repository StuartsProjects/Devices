/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 13/09/22

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This test program blinks the LED on the microcontroller PCB and external LEDs on
  pin A2. The buzzer on pin A3 is also sounded.

  Serial monitor baud rate is set at 9600.
*******************************************************************************************************/

#define LED1 8                                 //pin number for PCB LED, set logic level high for on
#define LED2 A2                                //pin number for External LED, set logic level high for on
#define BUZZ A3                                //pin number for Buzzer, set logic level high for on

uint16_t seconds;                              //used to display time elapsed on Serial Monitor

void loop()
{
  Serial.print(seconds);
  Serial.println(F(" Seconds"));               //this message should print on console at close to once per second
  seconds++;
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(BUZZ, HIGH);
  delay(100);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(BUZZ, LOW);
  delay(890);                                  //should give approx 1 second flash\buzz
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, HIGH);                  //LED on
    digitalWrite(LED2, HIGH);                  //Arduino board LED on
    delay(delaymS);
    digitalWrite(LED1, LOW);                   //LED off
    digitalWrite(LED2, LOW);                   //Arduino board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                       //setup pin as output for indicator LED
  pinMode(LED2, OUTPUT);                       //setup pin as output for some Arduino boards that include an LED on pin 13
  pinMode(BUZZ, OUTPUT);
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(BUZZ, HIGH);

  led_Flash(2, 125);                           //two quick LED flashes to indicate program start

  Serial.begin(9600);
  Serial.println();
  Serial.print(__FILE__);
  Serial.println();

  Serial.println(F("1_Blink_and_Buzz Starting"));
}
