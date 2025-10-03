This is a repository for LoRa test transmitter and receiver programs using the RadioLib Library. 

I was interested in giving Meshtastic a try, it uses LoRa after all. 

In the UK Meshtastic is used on the 868Mhz band and I already had a few of the Lilygo T3-S3 868Mhz boards which are supported for Meshtastic. I was also interested in testing the newest LoRa device, the LR1121 to see if there was any sensitivity differences between that and the older SX127X and SX126X LoRa devices, there is a LR1121 version of the T3-S3.

The first programs are;

**1\_Basic\_LoRa\_Transmitter**

The objective behind these two programs was to provide an easy to use and configure pair of programs that could be used for testing of receivers, transmitter, antennas and distances. 

You will need to download and install the following libraries;

RadioLib for LoRa -  [https://github.com/jgromes/RadioLib](https://github.com/jgromes/RadioLib "https://github.com/jgromes/RadioLib")

U8g2 library for OLED display - [https://github.com/olikraus/U8g2_Arduino](https://github.com/olikraus/U8g2_Arduino "https://github.com/olikraus/U8g2_Arduino")

The programs were tested on the Arduino IDE version 2.3.6 with version 3.2.0 of the Expressif ESP32 Core for the IDE.

To use these programs you will need to be familiar with using and setting up the Arduino IDE for the ESP32, its beyond the scope of this documentation to provide detailed instructions for using and setting up the IDE. 

### 1\_Basic\_LoRa\_Transmitter

This program transmits packets that are formed in an array called TXbuff. The format of the packet is ASCII text, LoRaxxxxx, where xxxxx is a number starting from 0 and incrementing to 65535 and looping back to 0. The number increases by one at each transmission. 

Within the program the LoRa frequency and modem settings are entered in the Settings.h file. The settings below are those used for testing some antennas. They are the Meshtastic Long Fast settings, with a slightly different frequency used to avoid potential problems with any nearby Meshtastic devices.

    Frequency = 868.0Mhz
    Bandwidth = 250Khz
    SpreadingFactor = 11
    CodeRate = 5 (4:5)
    SyncWord = 0x2B
    TXpower = 10 
    PreambleLength = 16

Also within the Settings.h file the various options for the examples can be configured by 'commenting in' the #includes. For example if the Settings.h file is edited so that it looks like this;

    #define PRINT_ASCII      //enable to print ASCII of packet
    //#define PRINT_HEX      //enable to print HEX of packet
    #define USE_DISPLAY      //enable OLED  
    #define USE_SD           //enable SD card for logging
    //#define USE_BLUETOOTH  //Requires an ESP32 processor and version 3.0.0 and above of Expressif ESP32 Core for IDE.
    //#define USE_BUZZER     //on GPIO10, pin 3 of P3, connector nearest long edge of board, receiver only

And you then configure for the selected LoRa device by editing an included file called Lilygo_T3S3.h which contains the pin definitions used by the LoRa device and the type of LoRa device also, see below; 

    //#define USE_LR1121     //enable this define if using an LR1121 LoRa device 
    #define USE_SX1262       //enable this define if using an SX1262 LoRa device 

With the above settings the program will compile for the SX1262 and use the OLED and the SD card for logging. The packets received will be printed in ASCII on the serial monitor and OLED, with no HEX printing and Bluetooth output is not enabled. 

Note that the transmit and receive programs do share a common set of variable definitions at the beginning of the sketch and in the setup() function, so it's possible there are some variables that are not actually used for each particular sketch. This was done to make it easier to adapt the programs for further purposes.  

Although the basic example sends a short ASCII text packet you could modify the programs to send longer or binary\HEX packets. However be aware the OLED display might get messed up if long or non-ASCII packets are sent. This also applies to the Serial monitor prints, sending non-ASCII packets could result in weird outputs on the Serial monitor. You can turn off the printing and display of ASCII packets by commenting out the PRINT_ASCII define in Settings.h 

**2\_Basic\_LoRa\_Receiver**

For the receiver to receive packets from the 1\_Basic\_LoRa\_Transmitter then the LoRa settings in Settings.h must match. 

Most of the configuration information above for 1\_Basic\_LoRa\_Transmitter also applies to this receiver program.

The receiver has the option of logging the similar output that goes to the Serial monitor to a log file created on the and SD card if found. The Serial monitor and attached OLED will display the name of the log file to be used. 

A practical example of using these programs to compare various 868Mhz antennas can be found on the blog here;

 

October 2025
