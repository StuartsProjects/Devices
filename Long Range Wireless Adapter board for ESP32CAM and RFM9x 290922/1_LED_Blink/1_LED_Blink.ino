/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/01/23

  This programs is supplied as is, it is up to the user of the program to decide if the programs are
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This program blinks the White and Red LEDs on an ESP32CAM on pins 4 and 33
  respectivly. Note that for many of the LoRa programs for the ESP32CAM board the white LED on pin 4
  needs to be disabled.
  
  Serial monitor baud rate is set at 115200.
*******************************************************************************************************/

#define WHITELED 4                             //pin number for White LED, set logic level high for on
#define REDLED 33                              //pin number for Red LED, set logic level low for on

uint16_t seconds;                              //used to display time elapsed on Serial Monitor


void loop()
{
  Serial.print(seconds);
  Serial.println(F(" Seconds"));               //this message should print on console at close to once per second
  seconds++;
  digitalWrite(WHITELED, HIGH);
  digitalWrite(REDLED, LOW);
  delay(100);
  digitalWrite(WHITELED, LOW);
  digitalWrite(REDLED, HIGH);
  delay(900);                                  //should give approx 1 second flash
}


void setup()
{
  pinMode(WHITELED, OUTPUT);                   
  pinMode(REDLED, OUTPUT);
  
  Serial.begin(115200);
  Serial.println();
  Serial.print(__FILE__);
  Serial.println();

  Serial.println(F("1_LED_Blink Starting"));
}
