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
#include <RTClib.h>
#include "ph.h"

void(* resetFunc) (void) = 0;     // Reset frunction

BluetoothModule bt;     // Create bt instance
SDAccess sd;            // create SD card instance
RTC_DS3231 rtc;         // Create RTC instance

PH ph(A0, 8, &rtc);     // Create Ph Controller - Inputs = (PH Input Pin, C02 Relay Trigger Pin, Pointer to RTC)

// PWM and LEDS
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();  // Setup PWM Driver
Light light(&pwm, &rtc);  // Setup my light, needs the driver and the time

// Temp Sensors
OneWire oneWire(2);                            // Setup onewire connection for comms with temp sensor(s)
DallasTemperature tempSensors(&oneWire);    // tell temp sensor library to use oneWire to talk to sensors
Temp temp(&tempSensors);                    // Tell the temp control library which sensors to use

EepromAccess eeprom(ph.getDataAddress(), light.getDataAddress(), temp.getDataAddress());   // Create eepromAccess class, send it a reference of everything that needs saved

// constants for seconds in standard units of time
const unsigned long SecondsPerHour = 60UL * 60;
const unsigned long SecondsPerMinute = 60;

void setup() {
  Serial.begin(9600);

  setupRTC();         // setup routine, gets time from RTC and sets it in the sketch
  eeprom.setup();      // Check for existing save, load if found, else generate new save and populate with default values
  bt.setup();          // init bluetooth comms
  //sd.init();          // init sd card, TODO: if card not present dont try to log
  light.init();       // set initial state and begin running routines
  temp.init();
  //ph.setup();


  Serial.println(F("Welcome to Aquatroller!"));
}

void loop() {

  unsigned long currentTime;
  currentTime = millis();

  // This is a non blocking bluetooth implementation. Thanks to Robin2's mega post for most of this code
  bt.loop();          // Check for and save valid packets
  if (bt.newParse) {              // If we have a new parsed packet
    decodePacket(bt.parsedData);  // Decode and perform correct call
    bt.newParse = false;          // Set to false so we can get a new packet
  }





  //temp.loop(currentTime);
  light.loop(getTimeInSeconds(0, 0, 0));  // Run light controls, it needs to know the current time
  //ph.loop(currentTime);
  eeprom.loop();

  rtc.now();


}

// TODO: Testing to verify it all work
void decodePacket(BTParse data) { // Decides which actions should be taken on input packet

  switch (data.primary) {
    ////////// EEPROM Actions //////////////////
    case 0: // EEPROM
      switch (data.option) {
        case 0: // Get Data from EEPROM
          Serial.println(F("Getting Data from EEPROM"));
          eeprom.getSettings();
          break;
        case 1: // Save Data to EEPROM
          Serial.println(F("Saving Data to EEPROM"));
          eeprom.updateSettings();
          break;
        case 2: // Reset EEPROM
          Serial.println(F("Ressetting Data on EEPROM"));
          eeprom.resetEeprom();
          break;
      }
      break;
    /////////////// LED Actions /////////////////
    case 1: // LED's
      switch (data.option) {
        case 0: // Get Target LED Brightness
          Serial.print(F("Target Brightness: "));                   //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          //light.getBright(data.subOption);
          break;
        case 1: // Set target brightness                            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
          // light.setBright((data.value, bt.parsedData.subOption);
          break;
        case 2: // Get Light Mode
          Serial.print(F("Light Mode: "));
          Serial.println(light.getMode());
          break;
        case 3: // Set Light Mode
          light.setMode(data.value);
          break;
        case 4: // Get LED On Time
          Serial.print(F("Light on time: "));
          Serial.println(light.getOnTime());
          break;
        case 5: // Set LED on time
          light.setOnTime(data.value);
          break;
        case 6: // Get LED Off Time
          Serial.print(F("Light off time: "));
          Serial.println(light.getOffTime(data.subOption));
          break;
        case 7: // Set LED Off time
          light.setOffTime(data.value);
          break;

      }
      break;
    /////////////// SD Card Actions ////////////////
    case 2:
      break;
    /////////////// Bluetooth Actions //////////////
    case 3:
      break;
    /////////////// PH Actions /////////////////////
    case 4:
      switch (data.option) {
        case 0: // Get target PH
          Serial.print(F("Target PH: "));
          Serial.println(ph.getTargetPh());
          break;
        case 1: // Set Target PH
          ph.setTargetPh(data.values.fValue);
          break;
        case 2: // Get Heater delay time
          Serial.print(F("PH Reading Delay: "));
          Serial.println(ph.getPhDelay());
          break;
        case 3: // Set heater delay
          ph.setPhDelay(data.values.lValue);
          break;
        case 4: // Get C02 PH Target
          Serial.print(F("PH Target - C02: "));
          Serial.println(ph.getC02PhTarget());
          break;
        case 5: // Set C02 PH Target
          ph.setC02PhTarget(data.values.fValue);
          break;
        case 6: // Get target PH drop with c02
          Serial.print(F("Target PH Drop with C02: "));
          Serial.println(ph.getTargetPhDrop());
          break;
        case 7: // Set target PH drop with c02
          ph.setTargetPhDrop(data.values.fValue);
          break;
        case 8: // Get target PPM C02
          Serial.print(F("Target PPM C02: "));
          Serial.println(ph.getTargetPPMC02());
          break;
        case 9: // Set PPM C02
          ph.setTargetPPMC02(data.values.iValue);
          break;
        case 10: // Get Kh Hardness
          Serial.print(F("Kh Hardness: "));
          Serial.println(ph.getKhHardness());
          break;
        case 11: // Set Kh hardness
          ph.setKhHardness(data.values.fValue);
          break;
        case 12: // Get c02 on time
          Serial.print(F("C02 on time: "));
          Serial.println(ph.getC02OnTime());
          break;
        case 13: // Set c02 on time
          ph.setC02OnTime(data.values.lValue);
          break;
      }
      break;
    //////////////// Temperature Actions ///////////////////////
    case 5: // Temperature
      switch (data.option) {

        case 0: // Get target temperature
          Serial.print(F("Target Temp: "));
          Serial.println(temp.getTargetTemp());
          break;
        case 1: // Set Target Temp
          temp.setTargetTemp(data.values.fValue);
          break;
        case 2: // Get Heater delay time
          Serial.print(F("Heater Delay: "));
          Serial.println(temp.getHeaterDelay());
          break;
        case 3: // Set heater delay time
          temp.setHeaterDelay(data.values.lValue);
          break;
        case 4:  // Get Temperature check delay
          Serial.print(F("Temperature Delay: "));
          Serial.println(temp.getTempDelay());
          break;
        case 5:  // Set temperature check delay
          temp.setTempDelay(data.values.lValue);
          break;
        case 6:
          Serial.print(F("Target Temperature: "));
          Serial.println(temp.getTargetTemp());
          break;
        case 7:
          temp.setTargetTemp(data.values.fValue);
          break;
      }
      break;
    //////////// Time Actions ////////////////////////
    case 6:
      switch (data.option) {
        case 0: // Get Current Time in SSM TODO: Testing !!!!!!!!!!!!!!
          Serial.print(F("Seconds Since Midnight: "));
          Serial.println(getTimeInSeconds(0, 0, 0));
          Serial.print(F("On Time: "));
          Serial.println(light.getOnTime());
          Serial.print(F("Off Time: "));
          Serial.println(light.getOffTime(0));

          break;
      }
      break;
    //////////// Soft Reset /////////////////////////
    case 9:
      if (data.option == 1 && data.subOption == 1 && data.values.iValue == 1) {
        resetFunc();

      }
  }
}



void setupRTC() {

  if (! rtc.begin()) {                      // try to startup RTC
    Serial.println(F("Couldn't find RTC"));
  } else {
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
