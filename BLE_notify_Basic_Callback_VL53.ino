/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

byte flags = 0b00111110;
//byte bpm;
byte heart[8] = {0,0, 0, 0, 0 , 0, 0, 0 };//0b00001110
//byte hrmPos[1] = { 2 };
uint16_t intValue = 245;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;
uint16_t Distance = 0;
uint16_t txValue = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

/////////////////////////////////////////////////
//BASE VL53 FOR ESP32

//SCL = D22
//SDA = D21

#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
///////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("WSoft");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  pServer->getAdvertising()->addServiceUUID(SERVICE_UUID); //important added line allows connection notfications

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  ////////////////////////////////////////////////////////
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
	  Serial.println(F("Failed to boot VL53L0X"));
	  while (1);
  }
  // power 
  Serial.println(F("VL53L0X API Simple Ranging example\n\n"));
  ///////////////////////////////////////////////////////
}

void loop() {	

	VL53L0X_RangingMeasurementData_t measure;
	lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
	if (measure.RangeStatus != 4) {  // phase failures have incorrect data
		
		Distance = measure.RangeMilliMeter;
		
		
	}
	else {
		Serial.println(" out of range ");
	}

	



  if (deviceConnected) {
   	
	txValue = Distance;
	intValue = txValue;	
	Serial.print("txValue	");
	Serial.println(txValue);

	uint16_t loWord; 
	byte loByte;
	byte hiByte;
	hiByte = highByte(intValue);
	loByte = lowByte(intValue);
	heart[0] = hiByte;
	heart[1] = loByte;	
	pCharacteristic->setValue(heart, 2);	
	pCharacteristic->notify();
	/*
	Serial.print("intValue	 ");
	Serial.print(intValue, BIN);
	Serial.print("	");
	Serial.print(intValue, DEC);
	Serial.print("	");
	Serial.print(intValue, HEX);
	Serial.println("	");
	Serial.print("hiByte	");
	Serial.print(hiByte, BIN);
	Serial.print("		");
	Serial.print(hiByte, DEC);
	Serial.print("		");
	Serial.println(hiByte, HEX);
	Serial.print("loByte	");
	Serial.print(loByte, BIN);
	Serial.print("		");
	Serial.println(loByte, DEC);
	Serial.println("Start");
	Serial.print("heart [0]	");
	Serial.println(heart[0], BIN);
	Serial.print("heart [1]	");
	Serial.println(heart[1], BIN);
	Serial.print("heart [2]	");
	Serial.println(heart[2]);
	Serial.print("heart [3]	");
	Serial.println(heart[3]);
	Serial.print("heart [4]	");
	Serial.println(heart[4]);
	Serial.println("");
	//intValue++;
	*/
  }
  delay(1000);
}
