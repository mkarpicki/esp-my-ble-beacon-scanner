#include <Arduino.h>

#ifndef MyBLEBeacon_h
#define MyBLEBeacon_h
  
  class MyBLEBeacon
  {
    public:
      MyBLEBeacon();
      MyBLEBeacon(String, String, String, String);
      String address;
      String name;
      String RSSI;
      String txPower;  
  };

#endif
