#include <Config.h>
#include <WiFi.h>

//#include <time.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEEddystoneURL.h>
#include <BLEEddystoneTLM.h>
#include <BLEBeacon.h>
#include <Arduino.h>


#include "MyBLEBeacon.h";
#include "MyBLEQueue.h";


#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  15        /* Time ESP32 will go to sleep (in seconds) */
#define FAST_TIME_TO_SLEEP  10

#define D2 2
#define D4 4

RTC_DATA_ATTR int bootCount = 0;



const char* ssid = Config::ssid;
const char* password = Config::password;

const char **myBeacons = Config::beaconAddresses;
const unsigned int myBeaconsLen = Config::beaconAddressesLength;


const char* host = "api.thingspeak.com"; 
const int httpPort = 80;

const unsigned int channelID = Config::channelID;
const char* writeApiKey = Config::writeAPIKey;

const unsigned int isHomeChannelID = Config::isHomeChannelID;
const char* isHomeWriteApiKey = Config::isHomeWriteAPIKey;
const bool isHome = Config::isHome;


//const char* ntpServer = "pool.ntp.org";
//const long  gmtOffset_sec = 3600;
//const int   daylightOffset_sec = 3600;

int scanTime = 5; //In seconds
//int scanDelay = (1000 * 60); //In miliseconds

BLEScan* pBLEScan;
MyBLEQueue * myBLEQueue = new MyBLEQueue();

//MyBLEBeacon* foundBeacons[10];
//int foundBeaconsLen = 0;


boolean isKnownAddress(BLEAddress address)
{
   for (int counter = 0; counter < myBeaconsLen; counter++) 
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

      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str()); 
      
      if(isKnownAddress(advertisedDevice.getAddress())) 
      {

        String bleAddress = advertisedDevice.getAddress().toString().c_str();
        String bleName = "";
        //String bleRSSI = "" ;
        int bleRSSI = 0 ;
        String bleTxPower = ""; 
        
        Serial.println(F("Found KNOWN BLE"));
        Serial.println(advertisedDevice.getAddress().toString().c_str());
        
        if (advertisedDevice.haveName())
        {
          bleName = advertisedDevice.getName().c_str();
        }
  
        if (advertisedDevice.haveRSSI())
        {
          //bleRSSI = String( advertisedDevice.getRSSI() );
          bleRSSI = advertisedDevice.getRSSI();
        }      
  
        if (advertisedDevice.haveTXPower())
        {
          bleTxPower = String(advertisedDevice.getTXPower());
        }

        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str()); 

        // write to the ThingSpeak channel
        //foundBeacons[foundBeaconsLen] = new MyBLEBeacon(bleAddress, bleName, bleRSSI, bleTxPower);
        //foundBeaconsLen++;
        
        myBLEQueue->push(
            new MyBLEBeacon(bleAddress, bleName, bleRSSI, bleTxPower)  
        );
      }
    }
};


void send(String bleAddress, String bleName, int bleRSSI, String bleTxPower)
{
  WiFiClient client;
  String footer = String(" HTTP/1.1\r\n") + "Host: " + String(host) + "\r\n" + "Connection: close\r\n\r\n";

  if (!client.connect(host, httpPort)) {
    return;
  }

  //Serial.println(ESP.getEfuseMac());
        
  String fieldScannerId = "&field1=" + WiFi.macAddress();
  String fieldBeaconMacAddress = "&field2=" + bleAddress;
  String fieldBeaconRSSI = "&field3=" + String(bleRSSI);
  String fieldBeaconName = "&field4=" + bleName;
  String fieldTxPower = "&field5=" + bleTxPower;
  String fieldTimestamp = "&field6=";

//  unsigned int homePosition = 0;
//  const int estimatedOneMeterSignal = -62;

  String query = String("GET /update?api_key=" + String(writeApiKey) + fieldScannerId + fieldBeaconMacAddress + fieldBeaconRSSI +fieldBeaconName + fieldTxPower + footer);

  Serial.println(query.c_str());

  client.print(query);
  readResponse(&client);

  if (isHome) {
    if (!client.connect(host, httpPort)) {
      return;
    }    
//    if (bleRSSI > estimatedOneMeterSignal) {
//      homePosition = 1;
//    }
    String isHomeQuery = String("GET /update?api_key=" + String(isHomeWriteApiKey) + "&field1=" + String(bleRSSI) + footer);
    Serial.println(isHomeQuery.c_str());
    
    client.print(isHomeQuery);
    readResponse(&client);
  }

}

tm getLocalTime()
{
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  return timeinfo;
}

int getLocalSeconds()
{
  int seconds = 0;  
  struct tm timeinfo = getLocalTime();
  if (&timeinfo != NULL)
  {
    seconds = timeinfo.tm_sec;
  }
  return seconds;
    
}

void ledPrepare() {
  pinMode(D2, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(D4, OUTPUT);
}

void ledOn() {
  digitalWrite(D2, HIGH);
  digitalWrite(D4, LOW);
}

void setup()
{

    Serial.begin(115200);
    while(!Serial){delay(100);}
    Serial.println("...[SETUP]...");
    Serial.print("isHome: ");
    Serial.println(isHome);
    Serial.println(WiFi.macAddress());

    ledPrepare();
    ledOn();
    

    // Init and get the time
    if (WiFiconnect())
    {
    
//      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
//      Serial.print("getLocalSeconds() : ");
//      Serial.println(getLocalSeconds());
//      //wait till equal time (sync many devices to send information around same second)
//      delay(
//        (60 - getLocalSeconds()) * 1000
//      );

      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    
      BLEDevice::init("");
      pBLEScan = BLEDevice::getScan(); //create new scan
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
      pBLEScan->setInterval(100);
      pBLEScan->setWindow(99);  // less or equal setInterval value
      
      BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
      //Serial.print(F("Devices found: "));
      //Serial.println(foundDevices.getCount());
      pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  
  
      MyBLEBeacon * beacon;
  
      while(!myBLEQueue->isEmpty())
      {
        beacon = myBLEQueue->pop();
        send(beacon->address, beacon->name, beacon->RSSI, beacon->txPower);
        delete beacon;        
      }
             
    }
    else
    {
      esp_sleep_enable_timer_wakeup(FAST_TIME_TO_SLEEP * uS_TO_S_FACTOR);
    }

    esp_deep_sleep_start();
}

void loop(){

    ledOn();
//  bool isConnected = true;
//  
//  if (WiFi.status() != WL_CONNECTED) {
//    if (!WiFiconnect()) {
//      isConnected = false;     
//    }    
//  }
//
//  if (isConnected) {
//
//    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//    //Serial.print(F("Devices found: "));
//    //Serial.println(foundDevices.getCount());
//    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
//
//
//    MyBLEBeacon * beacon;
//
//    while(!myBLEQueue->isEmpty())
//    {
//      beacon = myBLEQueue->pop();
//      send(beacon->address, beacon->name, beacon->RSSI, beacon->txPower);
//      delete beacon;        
//    }
//     
//  }
//  
//  delay(scanDelay);  
  
}
