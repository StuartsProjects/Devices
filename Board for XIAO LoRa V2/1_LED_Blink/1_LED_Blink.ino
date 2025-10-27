/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 15/11/22

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  The XIAO pin 13 LED and TX and RX LEDs are blinked. The blinks should be close to one per second.
  
  The serial prints are sent to the Serial port defined by MonitorPort. This can be Serial, the default
  which is the USB port or to Serial1 on pins A6 and A7.

  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#define TXLED 11                               //on board TX LED, blue
#define RXLED 12                               //on board RX LED, blue  
#define LED1 13                                //on board LED is yellow
#define MonitorPort Serial                     //can be changed to output serial prints on Serial or Serial1  

uint16_t counter;




void loop()
{
  counter++;

  MonitorPort.println(counter);

  digitalWrite(LED1, LOW);
  delay(100);
  digitalWrite(LED1, HIGH);
  delay(890);                                  //should give approx 1 second flash

  digitalWrite(TXLED, LOW);
  delay(100);
  digitalWrite(TXLED, HIGH);
  delay(890);

  digitalWrite(RXLED, LOW);
  delay(100);
  digitalWrite(RXLED, HIGH);
  delay(890);
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                    //board LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                   //board LED off
    delay(delaymS);
  }
}


void setup()
{
  pinMode(LED1, OUTPUT);                         //setup board LED pin as output
  digitalWrite(LED1, HIGH);                      //LED off
  pinMode(TXLED, OUTPUT);                        //setup board LED pin as output
  digitalWrite(TXLED, HIGH);                     //LED off
  pinMode(RXLED, OUTPUT);                        //setup board LED pin as output
  digitalWrite(RXLED, HIGH);                     //LED off

  led_Flash(2, 125);                             //2 LED flashes to indicate program start
  delay(2000);

  MonitorPort.begin(115200);
  MonitorPort.println();
  MonitorPort.println(F(__FILE__));
  MonitorPort.println();
}
