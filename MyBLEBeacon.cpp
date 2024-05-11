#include <Arduino.h>
#include "MyBLEBeacon.h";

MyBLEBeacon::MyBLEBeacon(void)
{
  address = "";
  name = "";
  RSSI = 0;
  txPower = "";
}

MyBLEBeacon::MyBLEBeacon(String address, String name, int RSSI, String txPower)
{
  this->address = address;
  this->name = name;
  this->RSSI = RSSI;
  this->txPower = txPower;
}
