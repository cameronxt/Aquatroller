#ifndef ph_h
#define ph_h

/************** Aquatroller PH and C02 Control Library *************************
 *******************************************************************************
 * This library is designed to allow you to monitor the PH inside your aquarium.
 * The value reported is actually an average of 10 readings spaced out over time,
 * and is the most recent reading. Using this PH reading we can then start adding 
 * c02 to the aquarium. This causes the PH to drop. By monitoring the ph before c02 
 * we can set a baseline to know how much the PH has dropped and by correlation, 
 * how much c02 has been added to the water. This code assumes a settable KH Hardness 
 * (in degrees) so please dont use PH altering products for the most accurate readings. 
 * to account for changes to the aquarium over time, we will remeasure the baseline 
 * every night. C02 is controlled via a relay, solid state preferred, connected to a 
 * 120v solenoid onthe c02 regulator. 
 * 
 * The loop() Function may be used several ways:
 * -The first is a usage example. This is the code i actually implement on my 
 *  controller so it should be functional as is. Once im done anyways :)
 * Should this loop not meet your requirements you have 2 options.
 * -Option 1: Dont use my loop, create your own, just remember to access everything 
 *  through your PH Object
 * - Option 2: Modify or rewrite my loop(). Downside to this is you lose your 
 *  modifications when you update this library.
 * 
 * TODO: Logic to control when c02 is allowed to be on, maybe pass
 * TODO: Calibration Logic and mode: Still need a way to input Target PH
 * TODO: Logic to control c02 based on PH
 * TODO: Log Data to SD Card
 *****************************************************************************
 *****************************************************************************
 */

#include <Arduino.h>
#include <DS3232RTC.h>                 // https://github.com/JChristensen/DS3232RTC
#include "RTClib.h"
#include <TimeLib.h>

const byte _bufSize = 10;    // Buffer size for PH values, 10 seems to work well 


 // Structure to store all user adjustable data for easier writing to EEPROM
struct PHData  {      
  int phPin = A0;   // PH Sensor Analog pin
  int C02Pin = 8;   // C02 relay pin
  byte bufSize = 10;    // Buffer size for PH values, 10 seems to work well
  
  unsigned long checkPhDelay = 1000;        // Wait one second between Checks, this allows sensor to stabilize
  unsigned long phStabilizeDelay = 1000 * 60 * 2;    // Delay to wait for ph to fully stabilize
  float restingPh;                            // PH after several hours without c02
  
  // Calibration Data
  float phCalValue;
  
  // C02 Data
  unsigned long checkC02Delay = 30000;        // Delay before cycling c02 relay in millis
  float targetPhC02;                          // PH target to know we have enough c02
  int targetPPMC02 = 20;                      // PPM of c02 that we are requesting
  float khHardness = 4.0;                     // KH hardness of your tank
};

class PH {

  PHData _phData;
  RTC_DS3231 *_rtc;         // Create RTC pointer

  public:
    
    PH(byte phInputPin, byte c02OutputPin, RTC_DS3231 *rtc);
    void init();
    void loop(unsigned long currentTime);

    // PH Methods
    void calibratePH(unsigned long currentTime, float* target);                     // PH Calibration Mode, gets calibration points
    void readPhRawToBuffer();                                                       // Adds a new ph reading to the buffer
    void processPhBuffer();                                                         // Average readings and convert to actual PH value
    
    void setTargetPH(float newTarget) { _targetPh = newTarget; };
    float getTargetPH() { return _targetPh; };
    
    void setPHDelay(unsigned long newDelay) { _phData.checkPhDelay = newDelay; };
    unsigned long getPHDelay() { return _phData.checkPhDelay; };
    
    float getCurrentPH() { return _currentPh; };
    
    void setCalPoints(float* calTarget, float* calActual);                          // Set the calibration points



    // C02 Methods
    void calculateRestingPh();                // PH after some time of c02 off, thinking maybe set 2 hours before light on
    void calculateTargetPh();                 // Calculate Target PH for needed c02 levels based on calculated resting ph
    int calculateC02PPM();                    // Calculate the PPM of c02 based on PH
    
    void turnOnC02() { digitalWrite(_c02Pin,LOW); };
    void turnOffC02() { digitalWrite(_c02Pin,HIGH); };

    float getRestingPh() { return _phData.restingPh; };
    void setRestingPh(float restingPh) { _phData.restingPh = restingPh; };
    
    float getC02PhTarget() { return _phData.targetPhC02; };
    void setC02PhTarget(float c02PhTarget) { _phData.targetPhC02 = c02PhTarget; };
    
    float getTargetPhDrop() { return _phData.targetPhC02; };
    void setTargetPhDrop(float targetPhC02) { _phData.targetPhC02 = targetPhC02; };
    
    int getTargetPPMC02 () { return _phData.targetPPMC02; };
    void setTargetPPMC02 (int targetPPM) { _phData.targetPPMC02 = targetPPM; };
    
    float getkhHardness () { return _phData.khHardness; };
    void setKhHardness (float khHardness) { _phData.khHardness = khHardness; };

  private:

    // Class Variables
    const int _phPin;   // PH Sensor Analog pin
    const int _c02Pin;   // C02 relay pin
    int _sensorValue = 0;             // Direct Sensor Value (0-1024)
    unsigned long _avgValue = 0;      // Calculated average
    unsigned long _prevPhTime = 0;    // When did we last read PH sensor
    int _buf[_bufSize], _temp;        // Buffer to average results. Size determined by const int _bufSize
    bool _calibrationMode = false;    // Flag to tell when we should be in PH calibration mode
    float _currentPh;                 // What is the current ph value
    int _phIndex = 0;                 // Index for PH buffer
    bool _newPh = true;               // Flag to tell when we have a new ph reading  
    bool _needRestingPh = false;      // Flag for when its time to get the resting ph
    bool _c02On = false;              // Flag for C02 being on
    int _currentC02PPM;               // Stores current ppm of c02 based on ph drop
    float _targetPh = 7.0;             // What PH are we targeting, will get changed based on c02 on or off
    unsigned long _prevC02Time = 0;

};

#endif
