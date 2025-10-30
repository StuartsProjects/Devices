/*******************************************************************************************************
  Programs for Arduino - Copyright of the author Stuart Robinson - 05/10/22

  This program is supplied as is, it is up to the user of the program to decide if the program is
  suitable for the intended purpose and free from errors.
*******************************************************************************************************/

/*******************************************************************************************************
  Tested on Seeeduino XIAO SAMD21.

  Program Operation - This is a simple program to test a GPS connected to a Seeeduino XIAO. The port
  normally used for I2C, pins A4 and A5 are re-configured to be a serial port. The program reads
  characters from the GPS using through a this serial port and sends (echoes) them to the IDE serial
  monitor. Connect the GPS TX pin to XIAO A5 (RX).

  Serial monitor baud rate is set at 115200.

*******************************************************************************************************/

#include <Arduino.h>
#include "wiring_private.h"                   //for pinPeripheral() function

Uart Serial3(&sercom2, A5, A4, SERCOM_RX_PAD_1, UART_TX_PAD_0);

volatile int serviceCount3 = 0;

extern "C"
{
  void SERCOM2_Handler(void)
  {
    serviceCount3++;                           //to check this handler is used
    Serial3.IrqHandler();
  }
}


void loop()
{
  if (Serial3.available())
  {
    Serial.write(Serial3.read());
  }
}

void setup()
{

  Serial.begin(115200);
  Serial.println();
  Serial.println(F(__FILE__));
  Serial.println();

  delay(2000);
  pinPeripheral(A3, PIO_SERCOM_ALT);
  pinPeripheral(A2, PIO_SERCOM_ALT);
  Serial3.begin(9600);                              //for GPS
}
