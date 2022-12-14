/*******************************************************************************
   Original code - Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

   Permission is hereby granted, free of charge, to anyone
   obtaining a copy of this document and accompanying files,
   to do whatever they want with them without any restriction,
   including, but not limited to, copying, modification and redistribution.
   NO WARRANTY OF ANY KIND IS PROVIDED.

   This uses OTAA (Over-the-air activation), where where a DevEUI and
   application key is configured, which are used in an over-the-air
   activation procedure where a DevAddr and session keys are
   assigned/generated for use with all further communication.

   Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
   g1, 0.1% in g2), as well as the TTN fair usage policy.

   To use this sketch, first register your application and device with
   the things network, to set or generate a DevAddr, NwkSKey and
   AppSKey. Each device should have their own unique values for these
   fields.

   Do not forget to define the radio type correctly in config.h.

 *******************************************************************************/

/*******************************************************************************

   modified by C. Pham for simple temperature sensor with low-power capability
   and support of single-channel gateway and 433MHz band
   Last update Feb 4th, 2020

 *******************************************************************************/

/*******************************************************************************

   Modified by Stuart Robinson December 2022 to provide working support for the
   Seeeduino XIAO SAMD21 and BME280 sensor board. This sketch needs the
   LMIC_low_power library;

   https://github.com/CongducPham/LMIC_low_power

   You MUST uncomment #define LMIC_LOWPOWER in the libraries lmic/src/lmic/config.h file

   Users of the sketch will need to be familiar with the operation and configuration
   of The Things Network (TTN), there is no detailed tutorial provided for the TTN part
   of the required configuration.

 *******************************************************************************/

#include <lmic.h>                     //get library here > https://github.com/CongducPham/LMIC_low_power
#include <hal/hal.h>
#include <SPI.h>
#include "ArduinoLowPower.h"          //get library here > https://github.com/arduino-libraries/ArduinoLowPower/blob/master/src/ArduinoLowPower.h

#include <Wire.h>
#include <Seeed_BME280.h>             //get library here; https://github.com/Seeed-Studio/Grove_BME280
BME280 bme280;                        //create an instance of the BME280 senosor

#include <CayenneLPP.h>               //Cayenne Low Power Payload (LPP)  > https://github.com/ElectronicCats/CayenneLPP
CayenneLPP lpp(51);

/********************************************************************
  _____              __ _                       _   _
  /  __ \            / _(_)                     | | (_)
  | /  \/ ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ _  ___  _ __
  | |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __| |/ _ \| '_ \
  | \__/\ (_) | | | | | | | (_| | |_| | | | (_| | |_| | (_) | | | |
  \____/\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__|_|\___/|_| |_|
                          __/ |
                         |___/
********************************************************************/

//for device eui-70b3d57ed0058404

// The APPEUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getArtEui (u1_t* buf)
{
  memcpy_P(buf, APPEUI, 8);
}

// The DEVEUI should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevEui (u1_t* buf)
{
  memcpy_P(buf, DEVEUI, 8);
}

// The APPKEY should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
// The key shown here is the semtech default key.

static const u1_t PROGMEM APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
void os_getDevKey (u1_t* buf)
{
  memcpy_P(buf, APPKEY, 16);
}

#define MonitorPort Serial1

#define DEBUG true                         //comment in this line to enable serialprints to the XIAO serial port, pin A6

#ifndef DEBUG
#define DEBUG false
#endif
#define Serial1 if(DEBUG)Serial1

//you MUST uncomment #define LMIC_LOWPOWER in your libraries/lmic/src/lmic/config.h file
// Schedule TX every this many seconds (might become longer due to duty
// cycle and fair use policy limitations).

const uint16_t TX_INTERVAL = 600;         //interval in seconds between payload transmissions, will be adjusted by FUP

const int16_t SPREADING_FACTOR = DR_SF12; //Define the intial data rate (SF) to use at startup

const uint8_t TX_POWER = 2;               //Define the transmit power in dBm to use at startup

#define SHOW_LMIC_LOWPOWER_TIMING         //comment in to show timing details

//Pins for Seeeduino XIAO SAMD21
const lmic_pinmap lmic_pins = {
  .nss = A3,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = A2,
  .dio = {A1, A1, LMIC_UNUSED_PIN},
};
const uint8_t LED1 = 13;

extern uint32_t os_cumulated_sleep_time_in_seconds;
uint32_t real_os_cumulated_sleep_time_in_seconds;

uint8_t payload[16];                 //maximum size of payload to send
uint32_t nextTransmissionTime = 0L;
uint32_t sentpayloads;
uint32_t upsecs;
uint32_t awakems;
uint32_t onairms;
uint16_t maxpayloadsaday;
uint32_t DUTY_CYCLE_INTERVAL;
uint32_t SLEEP_SECONDS;
static osjob_t sendjob;
ostime_t t_queued, t_complete;


void loop()
{
  os_runloop_once();
}


/*********************************************************************************
  Begin sensor code for BME280
*********************************************************************************/

void init_Payload()
{
  uint8_t index;
  bme280.init();
  for (index = 1; index <= 3; index++)
  {
    fill_Payload();                     //do an initial sensor read
    delay(250);
  }
}

void fill_Payload()
{
  uint8_t index;
  const uint8_t BME280ADDRESS = 0x76;   //default address of BME280 sensor

  float temperature;                    //Variable for temperature
  uint16_t humidity;                    //Variable for humidity, needs to be float for Cayenne
  float pressure;                       //Variable for pressure

  Wire.beginTransmission((uint8_t) BME280ADDRESS);
  Wire.write((uint8_t)0xF4);
  Wire.write((uint8_t)B01111111);
  Wire.endTransmission();

  temperature = bme280.getTemperature();
  pressure = bme280.getPressure() / 100.0F;
  humidity = bme280.getHumidity();

  //sensor can sometimes fail to respond, returning zero values, so re-initialize it
  if (pressure == 0)
  {
    MonitorPort.println(F("ERROR - BME280 failure"));
    bme280.init();

    for (index = 1; index <= 3; index++)
    {
      temperature = bme280.getTemperature();
      pressure = bme280.getPressure() / 100.0F;
      humidity = bme280.getHumidity();
      delay(250);
    }

    MonitorPort.println(F("BME280 restarted"));
  }

  MonitorPort.print(F("Temperature "));
  MonitorPort.print(temperature, 1);
  MonitorPort.println(F("C"));

  MonitorPort.print(F("Pressure "));
  MonitorPort.print(pressure, 0);
  MonitorPort.println(F("Pa"));

  MonitorPort.print(F("Humidity "));
  MonitorPort.print(humidity);
  MonitorPort.println(F("%"));

  lpp.reset();
  lpp.addTemperature(1, temperature);                //Add the current temperature into channel 1
  lpp.addBarometricPressure(2, pressure);            //Add the current pressure into channel 2
  lpp.addRelativeHumidity(3, (float) humidity);      //Add the current humidity into channel 3

  Wire.beginTransmission((uint8_t) BME280ADDRESS);
  Wire.write((uint8_t)0xF4);
  Wire.write((uint8_t)B01111100);
  Wire.endTransmission();
}

/*********************************************************************************
  End sensor code for BME280
*********************************************************************************/




void onEvent (ev_t ev)
{
  t_complete = os_getTime();

  switch (ev) {
    case EV_SCAN_TIMEOUT:
      MonitorPort.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      MonitorPort.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      MonitorPort.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      MonitorPort.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      MonitorPort.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      MonitorPort.println(F("EV_JOINED"));
      MonitorPort.print(F("netid: "));
      MonitorPort.println(LMIC.netid, DEC);
      MonitorPort.print(F("devaddr: "));
      MonitorPort.println(LMIC.devaddr, HEX);
      MonitorPort.print(F("artKey: "));
      for (int i = 0; i < sizeof(LMIC.artKey); i++) {
        if (i != 0)
          MonitorPort.print(F("-"));
        if (LMIC.artKey[i] < 16)
          MonitorPort.print(F("0"));
        MonitorPort.print(LMIC.artKey[i], HEX);
      }
      MonitorPort.println();
      MonitorPort.print(F("nwkKey: "));
      for (int i = 0; i < sizeof(LMIC.nwkKey); i++) {
        if (i != 0)
          MonitorPort.print(F("-"));
        if (LMIC.nwkKey[i] < 16)
          MonitorPort.print(F("0"));
        MonitorPort.print(LMIC.nwkKey[i], HEX);
      }
      MonitorPort.println();

      //Disable link check validation, automatically enabled during join'
      LMIC_setLinkCheckMode(0);
      break;
    case EV_RFU1:
      MonitorPort.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      MonitorPort.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      MonitorPort.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      {
        MonitorPort.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        sentpayloads++;
        MonitorPort.print(F("Sent payloads "));
        MonitorPort.println(sentpayloads);

        if (LMIC.txrxFlags & TXRX_ACK)
          MonitorPort.println(F("Received ack"));
        if (LMIC.dataLen) {
          MonitorPort.println(F("Received "));
          MonitorPort.println(LMIC.dataLen);
          MonitorPort.println(F(" bytes of payload"));
          for (int i = 0; i < LMIC.dataLen; i++) {
            if (LMIC.frame[LMIC.dataBeg + i] < 0x10) {
              MonitorPort.print(F("0"));
            }
            MonitorPort.print(LMIC.frame[LMIC.dataBeg + i], HEX);
          }
        }

        DUTY_CYCLE_INTERVAL = interval_Adjust(LMIC.dndr, sizeof(payload));

#ifdef SHOW_LMIC_LOWPOWER_TIMING
        ostime_t now = os_getTime();
        MonitorPort.print(F("millis: "));
        MonitorPort.println(millis());
        MonitorPort.print(F("ticks: "));
        MonitorPort.println((uint32_t)(us2osticks(micros()) + sec2osticks(os_cumulated_sleep_time_in_seconds)));
#endif

        MonitorPort.print(F("Cumulated duty cycle seconds: "));
        MonitorPort.println((uint32_t)os_cumulated_sleep_time_in_seconds);
        MonitorPort.print(F("Cumulated sleep seconds: "));
        MonitorPort.println((uint32_t)real_os_cumulated_sleep_time_in_seconds);
        real_os_cumulated_sleep_time_in_seconds = real_os_cumulated_sleep_time_in_seconds + TX_INTERVAL;
        delay(10);

        MonitorPort.print(F("Duty cycle interval: "));
        MonitorPort.println(DUTY_CYCLE_INTERVAL);
        MonitorPort.print(F("This awake mS: "));
        MonitorPort.println(millis() - awakems);
        MonitorPort.print(F("millis: "));
        MonitorPort.println(millis());
        MonitorPort.println(F("Sleeping"));
        MonitorPort.println();

        MonitorPort.flush();
        delay(10);
        digitalWrite(LED1, HIGH);                        //board LED off

        sleep_Seconds(SLEEP_SECONDS);                    //do board specific deep sleep stuff, on return board is awake

        awakems = millis();

        MonitorPort.println(F("Awake"));
        MonitorPort.print(F("millis: "));
        MonitorPort.println(millis());
        digitalWrite(LED1, LOW);                         //board LED on
        hal_sleep_lowpower(DUTY_CYCLE_INTERVAL);         //adjusts lmic_ticks to in effect bypass dutycycle
        hal_sleep_lowpower(25);                          //add a small bit extra to avoid wrap round issues

        do_send(&sendjob);
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(DUTY_CYCLE_INTERVAL), do_send);
        break;
      }
    case EV_LOST_TSYNC:
      MonitorPort.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      MonitorPort.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      //data received in ping slot
      MonitorPort.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      MonitorPort.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      MonitorPort.println(F("EV_LINK_ALIVE"));
      break;
    default:
      MonitorPort.println(F("Unknown event"));
      break;
  }
}


void do_send(osjob_t* j)
{
  fill_Payload();

  nextTransmissionTime = millis() + ((uint32_t) DUTY_CYCLE_INTERVAL * 1000);

  MonitorPort.print(F("Duty cycle interval: "));
  MonitorPort.println(DUTY_CYCLE_INTERVAL);

  MonitorPort.print(F("Next Transmission time millis(): "));
  MonitorPort.println(nextTransmissionTime);

  if (LMIC.opmode & OP_TXRXPEND)                   //Check if there is not a current TX/RX job running
  {
    MonitorPort.println(F("OP_TXRXPEND, not sending"));
  }
  else
  {
    //Prepare upstream data transmission at the next possible time.
#ifdef FORCE_SPREADING_FACTOR
    LMIC_setDrTxpow(SPREADING_FACTOR, TX_POWER);  //force SF12 for testing
#endif
    LMIC_setTxData2(1, lpp.getBuffer(), lpp.getSize(), 0);        //Prepare upstream data transmission at the next possible time.

    t_queued = os_getTime();
    MonitorPort.println(F("TX Packet queued"));
    MonitorPort.print(F("FREQ="));
    MonitorPort.println(LMIC.freq);
  }
}


uint32_t interval_Adjust(uint8_t sf_in_use, uint8_t payloadlength)
{
  //calculates an estimate of the packet to send airtime
  float tempfloat;
  uint32_t newinterval;

  switch (sf_in_use)
  {
    case 0:
      //SF12
      tempfloat = 1089 + (payloadlength * 27.31);
      MonitorPort.println(F("SF12 in use"));
      break;

    case 1:
      //SF11
      tempfloat = 545 + (payloadlength * 15.02);
      MonitorPort.println(F("SF11 in use"));
      break;

    case 2:
      //SF10
      tempfloat = 313 + (payloadlength * 8.19);
      MonitorPort.println(F("SF10 in use"));
      break;

    case 3:
      //SF9
      tempfloat = 157 + (payloadlength * 4.52);
      MonitorPort.println(F("SF9 in use"));
      break;

    case 4:
      //SF8
      tempfloat = 89 + (payloadlength * 2.56);
      MonitorPort.println(F("SF8 in use"));
      break;

    case 5:
      //SF7
      tempfloat = 44 + (payloadlength * 1.48);
      MonitorPort.println(F("SF7 in use"));
      break;

    default:
      //SF12
      tempfloat = 1089 + (payloadlength * 27.31);
      MonitorPort.println(F("Default SF12 in use"));
      break;
  }

  onairms = tempfloat;

  MonitorPort.print(F("Packet on air mS: "));
  MonitorPort.println(onairms);
  maxpayloadsaday = (30000 / onairms);
  MonitorPort.print(F("Maximum Payloads a day: "));
  MonitorPort.println(maxpayloadsaday);
  newinterval = 86400 /  maxpayloadsaday;

  if ((sentpayloads > 5) && (TX_INTERVAL <  newinterval))
  {
    MonitorPort.print(F("TX interval "));
    MonitorPort.print(TX_INTERVAL);
    MonitorPort.println(F(" too short"));
    MonitorPort.print(F("Sleep seconds required : "));
    MonitorPort.println(newinterval);
    SLEEP_SECONDS = newinterval;
  }
  return (onairms / 10) + 30;                      //return 30 secs more than required duty cycle
}


void led_Flash(uint16_t flashes, uint16_t delaymS)
{
  uint16_t index;

  for (index = 1; index <= flashes; index++)
  {
    digitalWrite(LED1, LOW);                       //board LED on
    delay(delaymS);
    digitalWrite(LED1, HIGH);                      //board LED off
    delay(delaymS);
  }
}


void sleep_Seconds(uint32_t seconds)
{
  LowPower.sleep(seconds * 1000);
}


void setup()
{
  MonitorPort.begin(9600);
  MonitorPort.println(F("24_LMIC_low_power_OTAA_XIAO_SAMD21"));

  pinMode(LED1, OUTPUT);
  led_Flash(20, 125);                           //20 quick LED flashes to indicate program start

  digitalWrite(LED1, LOW);                      //board LED on

  pinMode(A0, INPUT_PULLUP);                    //unused pin to INPUT_PULLUP to reduce sleep current
  pinMode(A7, INPUT_PULLUP);                    //unused pin to INPUT_PULLUP to reduce sleep current
  pinMode(A9, INPUT_PULLUP);                    //pin to INPUT_PULLUP to reduce sleep current

  delay(1000);

  os_init();      // LMIC init

  LMIC_reset();   // Reset the MAC state. Session and pending data transfers will be discarded.

  //Let LMIC compensate for +/- 10% clock error
  //we take 10% error to better handle downlink messages
  LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);

  //Set up the channels used by the Things Network, which corresponds
  //to the defaults of most gateways. Without this, only three base
  //channels from the LoRaWAN specification are used, which certainly
  //works, so it is good for debugging, but can overload those
  //frequencies, so be sure to configure the full frequency range of
  //your network here (unless your network autoconfigures them).
  //Setting up channels should happen after LMIC_setSession, as that
  //configures the minimal channel set.

  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      //g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      //g-band

  //Set data rate and transmit power for intial uplink
  LMIC_setDrTxpow(SPREADING_FACTOR, TX_POWER);
  pinMode(A1, INPUT_PULLDOWN);          //prevent A1 floating due to use of diodes from DIO0 and DIO1    

  SLEEP_SECONDS = TX_INTERVAL;          //setup initial values
  DUTY_CYCLE_INTERVAL = TX_INTERVAL;    //setup initial values

  init_Payload();                       //setup sensor needed to fill payload

  do_send(&sendjob);                    //Start job
}
