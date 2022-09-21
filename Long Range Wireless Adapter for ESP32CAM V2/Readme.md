## Long Range Wireless Adapter for ESP32CAM with LoRa

This is an adapter board for the ESP32CAM with a fitted DRF1278F 434Mhz LoRa module.

A fuller description of the connections used and build of the board will be found at the link below;

**[Long Range Wireless Adapter for ESP32CAM with LoRa](https://stuartsprojects.github.io/2022/02/05/Long-Range-Wireless-Adapter-for-ESP32CAM.html)**

The LoRa module will allow the transfer of the images taken by the ESP32CAM over very much longer distances that would be possible with WiFi or Bluetooth. With a second Long Range Wireless Adapter board acting as the receiver the images can be automatically saved to an SD card or transferred via a serial link to a folder on a PC. 

The ESP32CAM is not provided.

The assembled board is shown below. There are some choices in how the board is built so soldering the pin headers, sockets, wires and antenna is left to the user. 

<br>
<p align="center">
  <img width="300"  src="/ESP32CAM_29.jpg">
</p>
<br>

<br>
<p align="center">
  <img width="450"  src="/ESP32CAM_28.jpg">
</p>
<br>

### Partial kit for Long Range Wireless Adapter board

Supplied with the board is enough parts to make a functional Long Range Wireless Adapter board. You will need to solder several components into place, using your your own solder. The DRF1278F LoRa module is already soldered in place on the board. The kit contains;

- PCB with fitted DRF1278F LoRa module
- 2 x 8 pin 0.1" header sockets
- 4 pin 0.1" pin header for programming connection
- 3 pin 0.1" pin header for Serial2 monitor connection
- IN5817 Diode
- JST battery lead
- Tie wrap for battery lead
- SMA antenna socket
- Antenna wire, 17cm
- Pin 33 wire, silicone.
- Boot switch


<br>
<p align="center">
  <img width="450"  src="/ESP32CAM_31.jpg">
</p>
<br>

You will need to remove the white LED (or its driver transistor) that is on GPIO4 as this pin is needed for the LoRa device. See component circled in red below;

<br>
<p align="center">
  <img width="250"  src="/ESP32CAM_GPIO4_Transistor.jpg">
</p>
<br>

There are Arduino programs provided that allow the assembled board to take pictures, save them to the SD card on the ESP32CAM and then transfer the image via LoRa to another remote Arduino acting as the receiver. The receiver can be another Long Range Wireless Adapter and ESP32CAM board or another Arduino. You will need to download an install a specific Arduino LoRa library.

There is an optional program provided that allows the receiver Arduino to transfer the received images across to a folder on a Windows PC using a serial link and the terminal program Tera Term.

Example programs are provided for carrying out basic tests on the assembled board, see the documentation link below. 

### Other optional components

The ESP32CAM is fitted with an AMS1117 regulator this has a fairly large drop-out voltage making operation from 3 x AAs or a single Lithium battery problematic. To resolve this situation the AMS1117 can be removed and an MCP1702 or HT7833 regulator fitted on the Long Range Wireless Adapter, see the build instructions for details.

There is a connector on the bottom of the board that can be used for a GPS which uses the Serial programming port.

Resistors R1 and R2 can be fitted and allows for reading the battery voltage. This might be useful if your also trying to use the Long Range Wireless Adapter as a GPS tracker with the GPS fitted to the 4 pin connector at the bottom of the PCB. However fitting R1 and R2 will interfere with the correct running of the SD card.

SW1 is a small switch that can e fitted which will disconnect the GPS, if fitted, temporarily from the UOR\RXD pin to allow program uploads to be carried out. if you cannot find a switch with the required 2mm pin spacing then a 0.1" pin header and jumper will just fit. For the GPS to operate the two bottom pins of the switch (nearest the GPS connector) need to be connected. To program the ESP32CAM either disconnect the GPS or remove the connection from the bottom two pins.  



### Stuart Robinson

### February 2022
