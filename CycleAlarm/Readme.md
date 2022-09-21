## Cycle Alarm Extender

This is the repository for the programs and files for the cycle alarm extender described in this article;


## Example programs

The programs in this repository are provided so that you can test the board you have built, the fully working transmitter and receiver programs are also provided.

### 1\_Blink\_and\_Buzz

Tests that the external LED and buzzer for the alarm receiver are working

### 2\_Battery\_Voltage\_Read\_Test

To save power when the transmitter is in sleep mode the resistor divider that is used to read the battery voltage is turned off with a MOSFET. This program tests the circuit is working and allows the voltage reference in the micro controller to be calibrated. 

### 3\_LoRa\_Transmitter

Simple LoRa transmitter to test the LoRa device is working. 

### 4\_LoRa\_Receiver

Simple LoRa receiver to test the LoRa device is working.

### 5\_Alarm\_Transmitter

The working transmitter program for the micro controller wired up to the alarm.

### 6\_Alarm\_Receiver

The working receiver program that will flash an LED and sound a buzzer when an alarm packet is received.

### 7\_Alarm\_Simulator\_Transmitter

Simulates the operation of the alarm transmitter and allows the receiver to be tested without the alarm horn sounding. 

### 8\_Lilygo\_TBeam\_LoRa\_Receiver

A functional receiver program for the Lilygo TBeam board. This board is ESP32 based and has a LoRa device, display and battery. 

 

<br>
<br>

#### Stuart Robinson
#### September 2022
