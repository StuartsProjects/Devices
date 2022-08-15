# EasyBuild ESP32 Board

This is a repository for the schematics and board files for an easy build portable hand held or bench based LoRa receiver that has enough resources to be used for receiving files and images over LoRa.

The board can be built with through hole components only, although there is the option of adding a surface mount SD card holder and a 2Kbyte or 8Kbyte FRAM. 

The micro controller used was the NodeMCU ESP32, this has plenty of memory, is ready built and low cost.


<br>
<p align="center">
  <img width="450"  src="images/EasyBuildESP32_Shield_1.jpg">
</p>
<br>



The board has connectors for;

1. Mikrobus compatible plug in boards, such as LoRa modules.
1. 128x64 OLED display.
1. External Battery pack with power switch.
1. 3 off WS2811 NeoPixel LEDs.
1. 3 off push button switches.
1. 2Kbyte or 8Kbyte FRAM.  
1. I2C external sensors.
1. I2C MPR121 3X4 Touch Keypad.
1. External GPS.
1. USB to Serial adapter for serial file uploads to a PC.
1. Bare RFM9x or SX1280 LoRa modules.
  

## Gerber files and Support

Its possible that at some time in the future I will sell these PCBs, but for now you can use the Gerber files included here to get your own boards made. Note the Gerber files have the original name for the board; NodeMCUShieldBasicWithLoRa.

The only condition of use for these files is that you do not get the boards made and sell them on to others. 

Also appreciate that I cannot provide support for using this board for applications other than the programs in the \examples folder. If the examples don't work for you its most likely you have a build or part error.


### Stuart Robinson

### August 2022

