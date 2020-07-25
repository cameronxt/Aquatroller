#ifndef ph_h
#define ph_h
#include <Arduino.h> 

const int _phPin = A0;   // PH Sensor Analog pin
const int _bufSize = 10;

struct PHData  {
    int sensorValue = 0;  // Direct Sensor Value (0-1024)
    unsigned long avgValue; // Calculated average
    unsigned long checkPhDelay = 1000;
    unsigned long prevPhTime = 0;
 //   unsigned long _currentTime;
    int buf[_bufSize], temp;          // buffer to average results
    int phIndex = 0;                  // Index for PH buffer
    float targetPh = 7.0;
    float currentPh;

    float calTarget[2];
    float calActual[2];
    bool calibrationMode = false;
};

class PH {
   
    PHData phData;
  

    
  public:


    PH();
      void init();
      void loop(unsigned long currentTime);
      void setTargetPH(float newTarget);
      float getTargetPH();
      void setPHDelay(unsigned long newDelay);
      unsigned long getPHDelay();
      float getCurrentPH();
      void calibratePH();      
  
     // Variables

   
  private:
    void setCalPoints(float calActual[]);

};

#endif
