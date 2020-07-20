// Need to implement -
//Look for device
//connect to device
//send data from sd logs

// This library sets newParse to true when there is new data that needs to be acted upon
//   this data can be be decoded to determine action required

#include "bluetooth.h"

BluetoothModule::BluetoothModule() {        // Constructor
  const byte numChars = 32;                 // incoming buffer
  bool newData = false;                  // do we need to process new data yet
  bool newParse = false;
}

void BluetoothModule::init() {              // Initialize a BT module
  Serial.print(F("Initializing BT module..."));
  Serial.println(F(" Done"));
}


void BluetoothModule::loop() {
  recievePacket();
  if (newData == true) {                    // If we have a new packet
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    //  getParsedData();
    newData = false;          // Set false so we dont keep processing the data
  }
}

void BluetoothModule::recievePacket() {     // constantly monitors for new packets, sets newData true when a packet has been recieved
  static boolean recvInProgress = false;    // true while recieving a packet, we only process one char per loop because serial is slow
  static byte ndx = 0;                      // index of buffer
  char rc;                                  // serial character recieved

  while (Serial.available() > 0 && newData == false) {        // While there is data and we arent working with it
    rc = Serial.read();                                         // Get char from Serial

    if (recvInProgress == true) {                               // Have we already started recieving a packet?
      if (rc != endMarker) {                                      // If we have recieved the end marker
        receivedChars[ndx] = rc;                                  // then store new char in buffer
        ndx++;                                                    // increase index so we can get the next char
        if (ndx >= numChars) {                                    // if index has reached the end of the buffer
          ndx = numChars - 1;                                     // then move back one position
        }
      }
      else {
        receivedChars[ndx] = '\0';                                // terminate the string
        recvInProgress = false;                                   // we are done recieving data
        ndx = 0;                                                  // reset the index
        newData = true;                                           // tell parseData that we are ready to parse the data
      }
    }

    else if (rc == startMarker) {                               // If char is our start marker, begin storing data
      recvInProgress = true;
    }
  }
}
void BluetoothModule::parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the system
  parsedData.primary = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  parsedData.option = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  parsedData.subOption = atoi(strtokIndx);
  strtokIndx = strtok(NULL, ",");
  parsedData.value = atoi(strtokIndx);

  //    integerFromPC = atoi(strtokIndx);     // convert this part to an integer
  //    strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC
  //    floatFromPC = atof(strtokIndx);     // convert this part to a float

  newParse = true;
}
