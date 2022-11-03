/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 02/11/22

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO RP2040.

  The 3 colour LED on XIAO RP2040, is blinked Red, Green Blue. The blinks should be close to one per
  second. Messages are sent to the Serial Monitor also.

  Uses the earlephilhower/arduino-pico: Raspberry Pi Pico Arduino core, for all RP2040 boards.
  Get it here > https://github.com/earlephilhower/arduino-pico
  Board selected is 'Seeed XIAO RP2040'

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

//red on board LED is pin 17, green 16, blue 25
#define RED1 17
#define GREEN1 16
#define BLUE1 25

uint16_t seconds;                              //used to display time elapsed on Serial Monitor


void loop()
{
  Serial.print(seconds);
  Serial.println(F(" Seconds"));               //this message should print on console at close to once per second
  seconds++;

  digitalWrite(RED1, LOW);
  delay(100);
  digitalWrite(RED1, HIGH);
  delay(890);                                  //should give approx 1 second flash

  Serial.println(F(" Seconds"));
  seconds++;

  digitalWrite(GREEN1, LOW);
  delay(100);
  digitalWrite(GREEN1, HIGH);
  delay(890);

  Serial.println(F(" Seconds"));
  seconds++;

  digitalWrite(BLUE1, LOW);
  delay(100);
  digitalWrite(BLUE1, HIGH);
  delay(890);
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  //general purpose routine for flashing LED as indicator
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(RED1, LOW);                      //Red LED on
    delay(delaymS);
    digitalWrite(RED1, HIGH);                     //Red LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(RED1, OUTPUT);                           //Red LED pin to output
  pinMode(GREEN1, OUTPUT);                         //Green LED pin to output
  pinMode(BLUE1, OUTPUT);                          //Blue LED pin to output

  digitalWrite(RED1, HIGH);                        //Red LED to off
  digitalWrite(GREEN1, HIGH);                      //Green LED to off
  digitalWrite(BLUE1, HIGH);                       //Blue LED to off

  led_Flash(2, 125);                               //2 quick LED flashes to indicate program start

  Serial.begin(115200);
  Serial.println();
  Serial.println(F("1_LED_Blink Starting"));
}
