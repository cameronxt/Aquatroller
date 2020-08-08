#ifndef ph_h
#define ph_h

/************** Aquatroller PH and C02 Control Library *************************
 *******************************************************************************
 * This library is designed to allow you to monitor the PH inside your aquarium.
 * The value reported is actually an average of 10 readings spaced out over time,
 * and is the most recent calculated reading. Using this PH reading we can then start 
 * adding  c02 to the aquarium. This causes the PH to drop. By monitoring the ph before 
 * c02 we can set a baseline to know how much the PH has dropped and by correlation, 
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

  float targetPh = 7.0;
  unsigned long checkPhDelay = 1000;        // Wait one second between Checks, this allows sensor to stabilize
  unsigned long phStabilizeDelay = 1000 * 60 * 2;    // Delay to wait for ph to fully stabilize
  float restingPh;                            // PH after several hours without c02
  
  // Calibration Data
  float phCalValue;
  
  // C02 Data
  unsigned long checkC02Delay = 30000;        // Delay before cycling c02 relay in millis
  float targetPhC02 = 7.0;                          // PH target to know we have enough c02
  int targetPPMC02 = 20;                      // PPM of c02 that we are requesting
  float khHardness = 4.0;                     // KH hardness of your tank
  unsigned long c02OnTime;
  unsigned long c02OffTime;
  unsigned long c02OffgasDelay = 1000 * 60 * 60 * 2;      // Offgas delay, starts at 2 hours, plan to learn this based off PH readings
  unsigned long c02BuildupTime = 1000 * 60 *30;           // How long before lights should we turn on C02
};

class PH {

  public: PHData _phData;
  RTC_DS3231 *_rtc;         // Create RTC pointer

  public:
    
    PH(byte phInputPin, byte c02OutputPin, RTC_DS3231 *rtc);
    void setup();
    void loop(unsigned long ssm);


    // PH Methods
    void calibratePH();                     // PH Calibration Mode, gets calibration points
    void readPhRawToBuffer();                                                       // Adds a new ph reading to the buffer
    void processPhBuffer();                                                         // Average readings and convert to actual PH value
    int getDataAddress() { return &_phData; };
    
    // PH getters and setters
    void setTargetPH(float newTarget) { _phData.targetPh = newTarget; };
    float getTargetPH() { return _phData.targetPh; };
    
    void setPHDelay(unsigned long newDelay) { _phData.checkPhDelay = newDelay; };
    unsigned long getPHDelay() { return _phData.checkPhDelay; };

    float getCurrentPH() { return _currentPh; };                                    // Get most recently calculated PH
    
    void setCalPoints();                          // Set the calibration points
    float calculateCalibration(){ _phData.phCalValue = (( _calTarget[0] - _calActual[0] ) + ( _calTarget[1] - _calActual[1] )) / 2; };
    void setCalTarget(int index, float calTarget) { _calTarget[index] = calTarget; };
    void setCalActual(int index, float calActual) { _calActual[index] = calActual; };


    // C02 Methods - May make its own class
    void calculateRestingPh();                // PH after some time of c02 off, thinking maybe set 2 hours before light on
    void calculateTargetPh();                 // Calculate Target PH for needed c02 levels based on calculated resting ph
    int calculateC02PPM();                    // Calculate the PPM of c02 based on PH
    
    void turnOnC02();
    void turnOffC02();

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

    unsigned long getC02OnTime() { return _phData.c02OnTime; } ;
    void setC02OnTime(unsigned long onTime) { _phData.c02OnTime; };

    unsigned long getC02OffTime() { return _phData.c02OffTime; } ;
    void setC02OffTime(unsigned long onTime) { _phData.c02OffTime; };

    bool isC02On() { return _c02On; };

  private:

    // Class Variables
    const int _phPin;   // PH Sensor Analog pin
    const int _c02Pin;   // C02 relay pin
    int _sensorValue;             // Direct Sensor Value (0-1024)
    unsigned long _avgValue;      // Calculated average
    unsigned long _prevPhTime;    // When did we last read PH sensor
    int _buf[_bufSize], _temp;        // Buffer to average results. Size determined by const int _bufSize
    bool _calibrationMode = false;    // Flag to tell when we should be in PH calibration mode
    float _calTarget[2];
    float _calActual[2];
    float _currentPh;                 // What is the current ph value
    int _phIndex;                 // Index for PH buffer
    bool _newPh = true;               // Flag to tell when we have a new ph reading  
    bool _needRestingPh = false;      // Flag for when its time to get the resting ph
    bool _c02On = false;              // Flag for C02 being on
    int _currentC02PPM;               // Stores current ppm of c02 based on ph drop
    //float _targetPh;                  // What PH are we targeting, will get changed based on c02 on or off
    unsigned long _prevC02Time;   // When did we last check c02 in millis
    bool _co2On = false;              // Flag to tell state of c02
   // int* dataPtr;

};

#endif
