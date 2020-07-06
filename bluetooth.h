// incase it gets added multiple times
#ifndef bluetooth_h
#define bluetooth_h
#include "Arduino.h"

//#include "eepromaccess.h"

const byte numChars = 32;

struct BTParse {
  byte type;
  byte option;
  byte subOption;
  byte value;
};

struct BTSettings {
  int txPin = 0;
  int rxPin = 1;


};

class BluetoothModule {
  public:
    BluetoothModule();
    void init();
    void loop();

    bool newParse;              // True when data has been parsed, but not retrieved
    BTParse parsedData;         // Struct for holding parsed data

  private:
    void recievePacket();
    void parseData();

    bool newData;               // True when we have a full packet to parse

    char receivedChars[numChars];       // Buffer for incoming data
    char tempChars[numChars];           // Temp buffer for strtok

    char startMarker = '<';
    char endMarker = '>';
};

#endif
