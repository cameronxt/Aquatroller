#ifndef ph_h
#define ph_h
#include <Arduino.h> 
const int pin = A0;   // PH Sensor Analog pin
const int bufSize = 10;

class PH {
    int sensorValue = 0;  // Direct Sensor Value (0-1024)
    unsigned long avgValue; // Calculated average
    unsigned long checkPhDelay = 1000;
    unsigned long prevPhTime = 0;
    unsigned long currentTime;
    int buf[bufSize], temp;          // buffer to average results
    int phIndex = 0;
    
  public:
    PH();
    void init();
    void loop(unsigned long CurrentTime);
    void setTargetPH(float newPH);
    float getTargetPH();
    void setPHDelay(unsigned long NewDelay);
    unsigned long getPHDelay();
    void getCurrentPH();
    void calibrate();             
  private:
};

#endif
