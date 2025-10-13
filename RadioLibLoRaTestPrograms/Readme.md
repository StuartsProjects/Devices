This is a repository for LoRa test transmitter and receiver programs using the RadioLib Library. 

I was interested in giving Meshtastic a try, it uses LoRa after all. 

In the UK Meshtastic is used on the 868Mhz band and I already had a few of the Lilygo T3-S3 868Mhz boards which are supported for Meshtastic. I was also interested in testing the newest LoRa device, the LR1121 to see if there was any sensitivity differences between that and the older SX127X and SX126X LoRa devices, there is a LR1121 version of the T3-S3.

So I developed two programs to provide an easy to use and configure pair of programs that could be used for testing of receivers, transmitter, antennas and distances. 

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

Also within the Settings.h file the various options for the programs can be configured by 'commenting in' the #includes. For example if the Settings.h file is edited so that it looks like this;

    #define PRINT_ASCII      //enable to print ASCII of packet
    //#define PRINT_HEX      //enable to print HEX of packet
    #define USE_DISPLAY      //enable OLED  
    #define USE_SD           //enable SD card for logging
    //#define USE_BLUETOOTH  //Requires an ESP32 processor and version 3.0.0 and above of Expressif ESP32 Core for IDE.
    //#define USE_BUZZER     //on GPIO10, pin 3 of P3, connector nearest long edge of board, receiver only

And you then configure for the selected LoRa device by editing an included file called Lilygo_T3S3.h which contains the pin definitions used by the LoRa device and the type of LoRa device also, see below; 

    //#define USE_LR1121     //enable this define if using an LR1121 LoRa device 
    #define USE_SX1262       //enable this define if using an SX1262 LoRa device 

Then with the above settings the program will compile for the SX1262, use the OLED for display and the SD card for logging. The packets received will be printed in ASCII on the serial monitor and OLED, with the HEX printing and Bluetooth output options disabled. 

Note that the transmit and receive programs do share a common set of variable definitions at the beginning of the sketch and in the setup() function, so it's possible there are some variables that are not actually used for each particular sketch. This was done to make it easier to adapt the programs for further purposes.  

Although the basic example sends a short ASCII text packet you could modify the programs to send longer or binary\HEX packets. However be aware the OLED display might get messed up if long or non-ASCII packets are sent. This also applies to the Serial monitor prints, sending non-ASCII packets could result in weird outputs on the Serial monitor. You can turn off the printing and display of ASCII packets by commenting out the PRINT_ASCII #define in Settings.h 

## 2\_Basic\_LoRa\_Receiver

For this receiver program to receive packets from the 1\_Basic\_LoRa\_Transmitter then the LoRa settings in Settings.h must match. 

Most of the configuration information above for 1\_Basic\_LoRa\_Transmitter also applies to this receiver program.

The receiver has the option of logging the similar output that goes to the Serial monitor to a log file created on the SD card if found. The Serial monitor and attached OLED will display the name of the log file to be used. 

A practical example of using these programs to compare various 868Mhz antennas can be found on the blog here;

[https://stuartsprojects.github.io/2025/10/01/LoRa-Antenna-Testing-for-868Mhz.html](https://stuartsprojects.github.io/2025/10/01/LoRa-Antenna-Testing-for-868Mhz.html "https://stuartsprojects.github.io/2025/10/01/LoRa-Antenna-Testing-for-868Mhz.html")

The simple transmit and receive programs above are all you might need to compare antennas, boards or power amplifiers. However if you want to measure how effective or how far a particular set-up might be, or the distances the LoRa packets might reach, you need more information. 

In a lot of cases you will want to test how good a particular set-up is at correctly receiving the very weakest packets that have travelled the furthest distance. 

Lets suppose you want to test how effective, or how much range\distance improves, if you use a signal amplifier on a LoRa receiver. You might assume that if the LoRa devices reports increased RSSI for a particular remote transmitter that reception has actually improved, but that wont always be the case because of the effect that the inevitable noise the amplifier has on the signal to noise ratio the LoRa receiver sees. You could see improved RSSI but worst reception and the receiver no longer picking up packets from some remote transmitters. 

It would be possible to arrange a series of remote transmitters that would send packets from say 50km, 60km and 70km etc. If you were only picking up the packets transmitted from 50km with a particular LoRa receiver and then when you added a signal amplifier you picked up the packets from the 70km transmitter, the amplifier is helping. However that is a difficult testing scenario to arrange in practice. 

Rather than changing the distance from transmitter to receiver we can simulate different distances by changing the transmit power. If receiver A can pick up packets transmitted at 10dBm but no lower power and receiver B can pick up 9dbm, 8dbm and 7dbm power packets, receiver B has a 3dBm greater sensitivity. This greater sensitivity will mean that receiver B can then also receive packets from a greater distance than receiver A. 

The SX1262 and LR1121 can transmit packets from 22dBm down to -9dBm in single dBm steps. You can arrange for the transmitted packet to contain the value of transmit power that was used to send it. So the transmitter program sends a series of packets at these powers;

20dBm
19dBm
18dBm
.
to
.
-8dBm
-9dBm

The packet sent is formatted in ASCII and contains the power level used to send it and the contents look like this;

Link+20
Link+19
Link+18
.
to
.
Link-08
Link-09

The receiver can then decode the packet and display and record the powers that were used to send the packets it receives.  

Here is a simple example of how the descending packet power process could be used to evaluate reception in a particular area. You arrange a transmitter, on a pole for instance, to send packets from 22dBm down to -9dBm. The receiver will display the ASCII content of the packets and you can thus tell how much power was used to transmit a particular packet. You travel around your area and by noting the weakest power packets received in each location you can have a map of likely reception in the area and how much link margin you have. If in a particular location you can receive packets down to 2dBm, you can be confident that packets sent 10dBm will be reliably received.

I used this descending power link testing back in 2014 to work out the potential long range capability of LoRa. I had a transmitter and receiver, with just basic 1\4 wave wire antennas. The test location was hilltop to hilltop over a distance of 40km. At the remote hilltop, 40km away, I was picking up LoRa packets the transmitter sent at 3dBm. Thus if the full LoRa module power of 20dBm was used the distance covered should be 280km. By switching to long range LoRa settings that distance should increase to 850km or so. Lots of people at the time thought it was just implausible that such a simple transmitter with basic antennas and low power could go that far, what rubbish. But eventually those type of distances were achieved.   

## 3\_LoRa\_Link\_Test\_Transmitter

This is the Link test program that sends a sequence of descending power packets. The start power is defined by StartTXpower and the end power by EndTXpower definitions in the Settings.h file.

At the start of the transmit packet loop a 'StartLinkTest' packet is sent, normally 'Link999' which the receiver will recognise. The power for this 'StartLinkTest' packet is defined by the 'TXpower' definition in Settings.h. Do check that the emitted power is legal for your part of the World.  


## 4\_LoRa\_Link\_Test\_Receiver

This is the receiver for the 3\_LoRa\_Link\_Test\_Transmitter program.

The receiver decodes each received packet and stores the number of each power level received in an array. When the 'Link999' 'StartLinkTest' packet is received the receiver prints out, to the Serial monitor and\or SD card log, the totals of each power level received, 22dBm to -9dBm, like this;

    22dBm,17  21dBm,17  20dBm,17  19dBm,17  18dBm,17  17dBm,17  16dBm,17  15dBm,17  14dBm,17  13dBm,16  12dBm,16  11dBm,16  10dBm,17  9dBm,17  8dBm,15  7dBm,15  6dBm,12  5dBm,5  4dBm,4  3dBm,1  2dBm,0  1dBm,0  0dBm,1  -1dBm,0  -2dBm,0  -3dBm,0  -4dBm,0  -5dBm,0  -6dBm,0  -7dBm,0  -8dBm,0  -9dBm,0  
    CSV,17,17,17,17,17,17,17,17,17,16,16,16,17,17,15,15,12,5,4,1,0,0,1,0,0,0,0,0,0,0,0,0  

Now the CSV line can be cut and pasted into a text file, Windows Notepad will do, then saved as a file with a .CSV extension. This CSV file can then be opened with a spreadsheet program, I used LibreOffice. The data from the CSV file can then be copied into the spreadsheet, 'Link Tester Graph' that will display the pasted data as a Graph.

The OLED display, if enabled, will print out the ASCII of the packet received and the number of cycle loops it has detected. 

## Buzzer
The receive programs can be fitted with a buzzer that sounds when a packet is received. The GPIO pin for the buzzer is defined in the Lilygo\_T3S3.h file and in the Settings.h file you will need to enable the USE\_BUZZER define. 

The buzzer can be particularly useful with the 4\_LoRa\_Link\_Test\_Receiver program since you can use it to count the beeps and thus work out at which power level reception stops. When the loop of sending descending power packets starts, there is a double beep, so if you then count the single beeps you know the power level at which reception stops. 

Have fun.

### Stuart Robinson

### October 2025
