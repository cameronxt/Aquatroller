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
 * The loop() Function may be used several ways. 
 * -The first is a usage example. This is the code i actually implement on my controller 
 * so it should be functional as is.
 * Should this loop not meet your requirements you have 2 options.
 * -Option 1: Dont use my loop, create your own, just remember to access everything 
 * through your PH Object
 * - Option 2: Modify or rewrite my loop(). Downside to this is you lose your modifications
 * when you update this library.
 * 
 * TODO: Logic to control when c02 is allowed to be on
 * TODO: Calibration Logic and mode
 * TODO: Logic to control c02 based on PH
 * TODO: Log Data to SD Card
 */

#include <Arduino.h>

const int _phPin = A0;   // PH Sensor Analog pin
const int _C02Pin = 8;
const int _bufSize = 10;

 // Structure to store all user adjustable data for easier writing to EEPROM
struct PHData  {                  
  unsigned long checkPhDelay = 1000;
  float targetPh = 7.0;
  // Calibration Data
  float calTarget[2];
  float calActual[2];
  // C02 Data
  float restingPh;
  float targetPhC02;
  int targetPPMC02;
  float khHardness;
};

class PH {

    PHData phData;



  public:


    PH();
    void init();
    void loop(unsigned long currentTime);
    
    void setTargetPH(float newTarget) { phData.targetPh = newTarget; };
    float getTargetPH() { return phData.targetPh; };
    void setPHDelay(unsigned long newDelay) { phData.checkPhDelay = newDelay; };
    unsigned long getPHDelay() { return phData.checkPhDelay; };
    float getCurrentPH() { return _currentPh; };
    void calibratePH();
    void readPhRawToBuffer();
    void processPhBuffer();

    // TODO: All c02 control functions
    void turnOnC02() { digitalWrite(_phPin,LOW); };
    void turnOffC02() { digitalWrite(_phPin,HIGH); };

    void calculateRestingPh();                              // PH after some time of c02 off, thinking maybe set 2 hours before light on
    void calculateTargetPh();                               // Calculate Target PH for needed c02 levels based on calculated resting ph

    float getRestingPh() { return phData.restingPh; };
    void setRestingPh(float restingPh) { phData.restingPh; };
    float getC02PhTarget() { return phData.targetPhC02; };
    void setC02PhTarget(float c02PhTarget) { phData.targetPhC02 = c02PhTarget; };
    float getTargetPhDrop() { return phData.targetPhC02; };
    void setTargetPPMC02 (int targetPPM) { phData.targetPPMC02 = targetPPM; };
    int getTargetPPMC02 () { return phData.targetPPMC02; };
    void setKhHardness (float khHardness) { phData.khHardness = khHardness; };
    float getkhHardness () { return phData.khHardness; };
    int calculateC02PPM();


  private:
    void setCalPoints(float calTarget[], float calActual[]);

    // Class Variables
    int _sensorValue = 0;             // Direct Sensor Value (0-1024)
    unsigned long _avgValue = 0;      // Calculated average
    unsigned long _prevPhTime = 0;    // When did we last read PH sensor
    int _buf[_bufSize], _temp;        // Buffer to average results. Size determined by const int _bufSize
    bool _calibrationMode = false;    // Flag to tell when we should be in PH calibration mode
    float _currentPh;
    int _phIndex = 0;                 // Index for PH buffer
    bool _newPh = true;               // Flag to tell when we have a new ph reading  
    bool _needRestingPh = false;      // Flag for when its time to get the resting ph
    bool _c02On = false;              // Flag for C02 being on
    int _currentC02PPM;               // Stores current ppm of c02 based on ph drop
};

#endif
