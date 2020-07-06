// 1.0.0 initial release - basic bluetooth, sd card and eeprom
// 1.0.1 adds basic RTC functionality
// 1.0.2 adds basic Temp sensor functionality

#include "eepromaccess.h"
#include "sdaccess.h"
#include "bluetooth.h"
#include "temp.h"
#include <DS3232RTC.h>                 // https://github.com/JChristensen/DS3232RTC
#include "RTClib.h"

EepromAccess eeprom;    // Create eeprom class
BluetoothModule bt;     // Create bt class
SDAccess sd;            // create SD card class
RTC_DS3231 rtc;

OneWire oneWire;
DallasTemperature tempSensors(&oneWire);
Temp temp(&tempSensors);


// constants for seconds in standard units of time

const unsigned long SecondsPerHour = 60UL * 60;
const unsigned long SecondsPerMinute = 60;

void setup() {
  Serial.begin(9600);
  setupRTC();
  eeprom.init();      // Check for existing save, load if found, else generate new save and populate with default values
  bt.init();          // init bluetooth
  sd.init();          // init sd card, if card not present dont try to log
}

void loop() {
  // Keep out of timers, this is a non blocking bluetooth implementation

  unsigned long currentTime = millis();
  bt.loop();          // Check for and save valid packets
  if (bt.newParse) {
    decodePacket(bt.parsedData);
    bt.newParse=false;
  }

  


    eeprom.loop();
    temp.loop(currentTime);
  // Timer function
  // unsigned long currentTime =
}


void decodePacket(BTParse data) { // Decides which actions should be taken on input

  switch (data.type) {
    case 0: // EEPROM
      break;
    case 1: // LED
      break;
    case 2: // SD Card
      break;
    case 3: // Bluetooth
      break;
  }
}

void setupRTC() {

  if (rtc.lostPower()) {                    // check if battery died on RTC, if so lets set it to something
    Serial.println(F("RTC lost power, lets set the time!"));

    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2019, 4, 30, 23, 52, 0));
  }
  setSyncProvider(RTC.get);         //Sets our time keeper as the RTC
  // setTime(RTC.get);              // Sets system time to RTC Time
  setSyncInterval(5);               // number of seconds to go before requesting re-sync
  if (timeStatus() != timeSet)
    Serial.println(F("Unable to sync with the RTC"));
  else
    Serial.println(F("RTC has set the system time"));
  if (! rtc.begin()) {                      // try to startup RTC
    Serial.println(F("Couldn't find RTC"));
    //while (1);                              // if you cant find it, fix and wait for reset
  }
}

// Returns time in seconds
// Takes int for hour and minute and seconds, returns UL seconds. Input zero in either spot to return just seconds for one
// IF all passed values are 0 it will give you the current time in seconds since midnight
unsigned long getTimeInSeconds(int hours, int minutes, int seconds) {
  if ((hours == 0) && (minutes == 0) && (seconds == 0)) {
    return ((hour() * SecondsPerHour) + (minute() * SecondsPerMinute) + second());
  } else {
    return ((hours * SecondsPerHour) + (minutes * SecondsPerMinute) + seconds);
  }
}

unsigned long getTimeInSeconds (unsigned long getTime) {         // Calculate seconds since midnight for timers
  return ((hour(getTime) * SecondsPerHour) + (minute(getTime) * 60) + second(getTime));

}
