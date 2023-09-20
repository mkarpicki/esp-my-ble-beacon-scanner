// ensure this library description is only included once
#ifndef Config_h
#define Config_h

// library interface description
class Config
{
  public:
    Config(void);
    static const char* ssid;
    static const char* password;

    static const unsigned long channelID;
    static const char* writeAPIKey;   
    static const unsigned long field; 

    static const unsigned int beaconAddressesLength;
    static const char* beaconAddresses [];
};

#endif