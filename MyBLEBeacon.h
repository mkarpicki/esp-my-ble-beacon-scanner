#include <Arduino.h>

#ifndef MyBLEBeacon_h
#define MyBLEBeacon_h
  
  class MyBLEBeacon
  {
    public:
      MyBLEBeacon();
      MyBLEBeacon(String, String, int, String);
      String address;
      String name;
      int RSSI;
      String txPower;  
  };

#endif
