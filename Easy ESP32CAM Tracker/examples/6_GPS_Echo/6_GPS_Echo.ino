/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 12/12/20

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Program Operation - This is a simple program to test a GPS connected to the Easy ESP32CAM board.
  The program reads characters from the GPS connected to the programming port and sends them (echoes)
  them to the Serial debug port on pin 33 of the ESP32CAM.

  Serial monitor baud rate is set at 115200.

*******************************************************************************************************/

#define WHITELED 4            //pin number for ESP32CAM on board white LED, set logic level high for on

#define RXD0 3                //RX pin for GPS port.
#define TXD0 1                //TX pin for GPS port

#define RXD2 40               //RX pin for Serial2, out of range so not in use  
#define TXD2 33               //TX pin for Serial2 

#define GPSserial Serial2     //the GPS port
#define GPSBAUD 9600          //default GPS baud rate

void loop()
{
  while (GPSserial.available())
  {
    Serial.write(GPSserial.read());
  }
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
  whiteFlash(5, 5, 195);

  GPSserial.begin(GPSBAUD, SERIAL_8N1, RXD0, TXD0);   //GPS port on programming port
  Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);       //format is Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.println();
  Serial.println();
  Serial.println("Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2)");
  Serial.println("6_GPS_Echo for Easy ESP32CAM Starting");
  Serial.println();

}
