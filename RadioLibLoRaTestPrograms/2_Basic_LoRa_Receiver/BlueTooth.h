/*
  Created Stuart Robinson 22/09/25
*/

/*
  Note: Requires version 3.0.0 plus of Arduino ESP32 core, version 3.3.0 used in this example
*/

#ifdef USE_BLUETOOTH

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool newMessage = false;
uint8_t txValue = 0;
bool mR = false;
String rxValue;

#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

String BLEDevice = "BLEserver";

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        mR = true;
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);

        Serial.println();
        newMessage = true;
        Serial.println("*********");
      }
    }
};


void BLE_setup()
{
  Serial.print(F("BLE_setup() "));
  Serial.println(BLEDevice);

  // Create the BLE Device
  BLEDevice::init(BLEDevice);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY);

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_RX,
      BLECharacteristic::PROPERTY_WRITE);

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println(F("Bluetooth - Waiting a client connection"));
}


void respond_int(int32_t value)
{
  String mystring;
  mystring = String(value);
  pTxCharacteristic->setValue(mystring);
  pTxCharacteristic->notify();
}


void respond_uint(uint32_t value)
{
  String mystring;
  mystring = String(value);
  pTxCharacteristic->setValue(mystring);
  pTxCharacteristic->notify();
}


void respond_float(float f, int places )
{
  String mystring;
  mystring = String(f, places);
  pTxCharacteristic->setValue(mystring);
  pTxCharacteristic->notify();
}


void respond(String send_message)
{
  pTxCharacteristic->setValue(send_message);
  pTxCharacteristic->notify();
}


void log_packetRXBluetooth(uint8_t *buff, int16_t lorastate, uint8_t rxpacketl, float packetrssi, float packetsnr)
{
  buff[rxpacketl] = 0;                 //manually set character after packet end to null for string conversion to work

  if (lorastate == RADIOLIB_ERR_NONE)
  {
    String str = (char*)buff;

    respond(str);

    //respond(",Bytes,");
    //respond_uint(rxpacketl);

    respond(",RSSI,");
    respond_int(packetrssi);
    //respond("dBm");

    respond(",SNR,");
    respond_int(packetsnr);
    //respond("dB");

  }
  else
  {
    if (lorastate == RADIOLIB_ERR_CRC_MISMATCH)
    {
      respond("CRCerror!");        //packet was received, but is malformed
    }
    else
    {
      respond("failed code ");       //some other error occurred
      respond_int(lorastate);
    }
  }
  respond("\n");
}


void log_packetTXBluetooth(uint8_t *buff, int16_t lorastate, uint8_t txpacketl)
{

  if (tempstate == RADIOLIB_ERR_NONE)
  {
    buff[txpacketl] = 0;                 //manually set character after packet end to null for string conversion to work

    String str = (char*)buff;

    respond(str);
  }
  else
  {
    if (lorastate == RADIOLIB_ERR_CRC_MISMATCH)
    {
      respond("TX Error!");        //error sending packet
    }
    else
    {
      respond("failed code ");       //some other error occurred
      respond_int(lorastate);
    }
  }
  respond("\n");
}


#endif
