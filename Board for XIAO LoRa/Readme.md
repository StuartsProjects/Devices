## Seeeduino XIAO SAMD21

This is the repository for programs for the Seeeduino XIAO SAMD21 version, some of which use the SX127X-LoRa library. The XIAO SAMD21 is discussed in this article;


[**https://stuartsprojects.github.io/2022/10/03/Seeeduino_XIAO.html**](https://stuartsprojects.github.io/2022/10/03/Seeeduino_XIAO.html)


Note that the Arduino program examples use DIO0 on the RFM9X LoRa module to sense TX and RX done. To ensure that this pin is connected to the XIAO A1 pin you will need to fit a wire link in the D4 position. 

### TTN LoRaWAN use

If you want to use the board for TTN LoRaWAN, there are two choices;

First you can fit diodes (1N4148) in the D4 and D2 positions and then configure the LMIC definition for the pins as;

    //Pins for Seeeduino XIAO SAMD21
    const lmic_pinmap lmic_pins = {
      .nss = A3,
      .rxtx = LMIC_UNUSED_PIN,
      .rst = A2,
      .dio = {A1, A1, LMIC_UNUSED_PIN},
    };

This sharing of DIO0 and DIO1 onto one pin, A1, with diodes seems to work well enough. 

Or if you wish you can connect DIO0 and DIO1 onto seperate pins of the XIAO with links. If you fit a wire link in the D4 position and fit a wire link on LK2 (in the middle of D2) you can use this LMIC pin definition;


    //Pins for Seeeduino XIAO SAMD21
    const lmic_pinmap lmic_pins = {
      .nss = A3,
      .rxtx = LMIC_UNUSED_PIN,
      .rst = A2,
      .dio = {A1, A6, LMIC_UNUSED_PIN},
    };

But note that you then loose the use of the serial port on the XIAO.

<br>
<br>

#### Stuart Robinson
#### October 2022
