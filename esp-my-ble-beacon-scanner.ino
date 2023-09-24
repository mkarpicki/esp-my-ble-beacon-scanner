#include <Config.h>
#include <WiFi.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <Arduino.h>


#include "MyBLEBeacon.h";
//#include "MyBLEQueue.h";


const char* ssid = Config::ssid;
const char* password = Config::password;

const char **myBeacons = Config::beaconAddresses;
const unsigned int myBeaconsLen = Config::beaconAddressesLength;

const char* host = "api.thingspeak.com"; 
const int httpPort = 80;
const unsigned int channelID = Config::channelID;
const char* writeApiKey = Config::writeAPIKey;

int scanTime = 5; //In seconds
int scanDelay = (3000 * 10); //In miliseconds
int counter;

BLEScan *pBLEScan;
MyBLEBeacon myBLEBeacon;
//MyBLEQueue * myBLEQueue = new MyBLEQueue();


boolean isKnownAddress(BLEAddress address)
{
   for (counter = 0; counter < myBeaconsLen; counter++) 
   {      
      if (strcmp (address.toString().c_str(), myBeacons[counter]) == 0) {
          return true;           
      }
   }
   return false;
}


bool WiFiconnect() {

  // Connect to Wifi.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println(F("Failed to connect to WiFi. Please verify credentials: "));
      delay(10000);
    }

    delay(500);
    Serial.println(F("..."));
    // Only try for 5 seconds.
    if (millis() - wifiConnectStart > 15000) {
      Serial.println(F("Failed to connect to WiFi"));
      return false;
    }
  }
  
  Serial.println(F("WiFi connected"));
  Serial.println(WiFi.localIP());

  return true;
}


void readResponse(WiFiClient *client){
  unsigned long timeout = millis();
  while(client->available() == 0){
    if(millis() - timeout > 5000){
      Serial.println(F("Client Timeout !"));
      client->stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while(client->available()) {
    String line = client->readStringUntil('\r');
    Serial.print(line);
  }

  Serial.printf("\nClosing connection\n\n");
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {

      if(isKnownAddress(advertisedDevice.getAddress())) 
      {

        String bleAddress = advertisedDevice.getAddress().toString().c_str();
        String bleName = "";
        String bleRSSI = "" ;
        String bleTxPower = ""; 
        

        Serial.println(F("Found BLE"));
        Serial.println(advertisedDevice.getAddress().toString().c_str());
        
        if (advertisedDevice.haveName())
        {
          bleName = advertisedDevice.getName().c_str();
          //Serial.print(F("Device name: "));
          //Serial.println(advertisedDevice.getName().c_str());
        }
  
        if (advertisedDevice.haveRSSI())
        {
          bleRSSI = String( advertisedDevice.getRSSI() );
          //Serial.print(F("Device RSSI: "));
          //Serial.println(advertisedDevice.getRSSI());
        }      
  
        if (advertisedDevice.haveTXPower())
        {
          bleTxPower = String(advertisedDevice.getTXPower());
          //Serial.print(F("Device TXPower: "));
          //Serial.println(advertisedDevice.getTXPower());
        }

        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str()); 

        // write to the ThingSpeak channel
        //send(bleAddress, bleName, bleRSSI, bleTxPower);

        // #hack: ugly hack for now (have a chance to work if only 1 BLE suppported)
        // should add to list
        myBLEBeacon.address = bleAddress;
        myBLEBeacon.name = bleName;
        myBLEBeacon.RSSI = bleRSSI;
        myBLEBeacon.txPower = bleTxPower;

//        Serial.println("PUSH");
//        
//        myBLEQueue->push(
//          new MyBLEBeacon(bleAddress, bleName, bleRSSI, bleTxPower) 
//        );
      }
    }
};


void send(String bleAddress, String bleName, String bleRSSI, String bleTxPower)
{
  WiFiClient client;
  String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";

  if (!client.connect(host, httpPort)) {
    return;
  }

  //Serial.print(F("Device address: "));
  //Serial.println(ESP.getEfuseMac());
        
  String fieldScannerId = "&field1=" + WiFi.macAddress();
  String fieldBeaconMacAddress = "&field2=" + bleAddress;
  String fieldBeaconRSSI = "&field3=" + bleRSSI;
  String fieldBeaconName = "&field4=" + bleName;
  String fieldTxPower = "&field5=" + bleTxPower;
  String fieldTimestamp = "&field6=";

  String query = String("GET /update?api_key=" + String(writeApiKey) + fieldScannerId + fieldBeaconMacAddress + fieldBeaconRSSI +fieldBeaconName + fieldTxPower + footer);

  Serial.println(query.c_str());

  client.print(query);
  readResponse(&client);

}


void setup()
{
    Serial.begin(115200);
    while(!Serial){delay(100);}
    Serial.println("...[SETUP]...");
  
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop(){

  bool isConnected = true;
  
  if (WiFi.status() != WL_CONNECTED) {
    if (!WiFiconnect()) {
      isConnected = false;     
    }    
  }

  if (isConnected) {

    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    //Serial.print(F("Devices found: "));
    //Serial.println(foundDevices.getCount());
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

    // #hack
    // should iterate on list
    if (myBLEBeacon.address) {
      send(myBLEBeacon.address, myBLEBeacon.name, myBLEBeacon.RSSI, myBLEBeacon.txPower);
    }

//    MyBLEBeacon * beacon;
//    Serial.println("QUEUE SIZE");
//    Serial.println(myBLEQueue->getSize());
//    while(!myBLEQueue->isEmpty())
//    {
//      beacon = myBLEQueue->pop();
//      Serial.println("POP");  
//      Serial.println(beacon->address);
////      send(beacon->address, beacon->name, beacon->RSSI, beacon->txPower);
////      Serial.println("SEND DONE\n\n\n\n\n");
////      //delete beacon;        
//    }
   
  }
  
  delay(scanDelay);  
  
}
