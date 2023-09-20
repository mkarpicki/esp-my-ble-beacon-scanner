#include "Config.h"

Config::Config(void)
{

};

const char* Config::ssid = "";
const char* Config::password = "";


const unsigned long Config::channelID = 0;
const char* Config::writeAPIKey = "";


const unsigned int Config::beaconAddressesLength = 0;
const char* Config::beaconAddresses[] = {
  "xx:xx:xx:xx:xx:xx"
};
