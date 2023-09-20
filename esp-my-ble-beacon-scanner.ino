#include <Config.h>



/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>


//#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

int scanTime = 5; //In seconds
int scanDelay = 5000; //In miliseconds
int counter;

BLEScan *pBLEScan;


const char **myBeacons = Config::beaconAddresses;
const unsigned int myBeaconsLen = Config::beaconAddressesLength;


boolean isKnownAddress(BLEAddress address)
{
   for (counter = 0; counter < myBeaconsLen; counter++) 
   {
//      Serial.print(address.toString().c_str());
//      Serial.print(" mine: ");
//      Serial.print(myBeacons[counter]);
//      Serial.println("");
//      
      if (strcmp (address.toString().c_str(), myBeacons[counter]) == 0) {
          return true;           
      }
   }
   return false;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {

      if(isKnownAddress(advertisedDevice.getAddress())) 
      {

        Serial.print("Device address: ");
        Serial.println(advertisedDevice.getAddress().toString().c_str());
        Serial.println("");
        
        if (advertisedDevice.haveName())
        {
          Serial.print("Device name: ");
          Serial.println(advertisedDevice.getName().c_str());
          Serial.println("");
        }
  
        if (advertisedDevice.haveRSSI())
        {
          Serial.print("Device RSSI: ");
          Serial.println(advertisedDevice.getRSSI());
          Serial.println("");
        }      
  
        if (advertisedDevice.haveTXPower())
        {
          Serial.print("Device TXPower: ");
          Serial.println(advertisedDevice.getTXPower());
          Serial.println("");
        }    
         
        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str()); 
        Serial.println("***************************************");
      }
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  
//  Serial.println("myBeaconsLen");
//  Serial.println(myBeaconsLen);


  delay(scanDelay);
}
