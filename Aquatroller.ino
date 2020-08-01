// 0.0.0 initial release - basic bluetooth, sd card and eeprom
// 0.0.1 adds basic RTC functionality
// 0.0.2 adds basic Temp sensor functionality
// 0.0.3 adds more basic Bluetooth, EEPROM access, SD Card access and PH control
// 0.0.4 adds basic LED control

#pragma GCC optimize ("-O2")   // Compiler Optimization Level

#include "eepromaccess.h"
#include "sdaccess.h"
#include "bluetooth.h"
#include "temp.h"
#include "lights.h"
#include <DS3232RTC.h>                 // https://github.com/JChristensen/DS3232RTC
//#include <RTClib.h>
#include "ph.h"

EepromAccess eeprom;    // Create eeprom instance
BluetoothModule bt;     // Create bt instance
SDAccess sd;            // create SD card instance
RTC_DS3231 rtc;         // Create RTC instance

PH phControl(A0, 8, &rtc);     // Create Ph Controller - Inputs = (PH Input Pin, C02 Relay Trigger Pin, Pointer to RTC)

// PWM and LEDS
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();  // Setup PWM Driver
Light light(&pwm, &rtc);  // Setup my light, needs the driver and the time

// Temp Sensors
OneWire oneWire;                            // Setup onewire connection for comms with temp sensor(s)
DallasTemperature tempSensors(&oneWire);    // tell temp sensor library to use oneWire to talk to sensors
Temp temp(&tempSensors);                    // Tell the temp control library which sensors to use


// constants for seconds in standard units of time
const unsigned long SecondsPerHour = 60UL * 60;
const unsigned long SecondsPerMinute = 60;

void setup() {
  Serial.begin(9600);
  setupRTC();         // setup routine, gets time from RTC and sets it in the sketch
  eeprom.init();      // Check for existing save, load if found, else generate new save and populate with default values
  bt.init();          // init bluetooth comms
  sd.init();          // init sd card, TODO: if card not present dont try to log
  light.init();       // set initial state and begin running routines
  phControl.init();
}

void loop() {
  // This is a non blocking bluetooth implementation. Thanks to Robin2's mega post for most of this code
  unsigned long currentTime = millis();
  bt.loop();          // Check for and save valid packets

  if (bt.newParse) {              // If we have a new parsed packet
    decodePacket(bt.parsedData);  // Decode and perform correct call
    bt.newParse = false;          // Set to false so we can get a new packet
  }





  temp.loop(currentTime);
  light.loop(getTimeInSeconds(0, 0, 0));  // Run light controls, it needs to know the current time
  phControl.loop(currentTime);
  eeprom.loop();
  // Timer functions
  // unsigned long currentTime =
}


void decodePacket(BTParse data) { // Decides which actions should be taken on input packet

  switch (data.primary) {
    case 0: // EEPROM
      break;
    case 1: // LED
      break;
    case 2: // SD Card
      break;
    case 3: // Bluetooth
      break;
    case 4: // PH
      switch (data.option) {
        case 0: // Target PH
          switch (data.subOption) {
            case 0:
              //ph.getTargetPH();
              break;

            case 1:
              //ph.setTargtPH(data.value);
              break;
          }
          break;
      }
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
