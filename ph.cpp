
#include "ph.h"

PH::PH() {
  
}

void PH::init() {
  // Set initial variables if we find them stored in eeprom
}

void PH::loop(unsigned long currentTime){

  // Timer so we only read ph every so often
  if (phData.prevPhTime - currentTime >= phData.checkPhDelay) {
    //Serial.println(F("its time to check it"));
    phData.buf[phData.phIndex]=analogRead(_phPin);               // store anolg value into buffer position
    phData.prevPhTime = millis();                         // Reset timer to allow ph sensor to stabilize before next reading
    if (phData.phIndex < _bufSize){                       // if not full advance to the next index
      phData.phIndex++;
    } else {                                        // If full calculate average value
      for(int i=0;i<(_bufSize-1);i++) {             // First lets sort our results
        for(int j=i+1;j<_bufSize;j++) {
          if(phData.buf[i]>phData.buf[j]) {
            phData.temp=phData.buf[i];
            phData.buf[i]=phData.buf[j];
            phData.buf[j]=phData.temp;
          }
        }
      }
      
       phData.avgValue=0;                                         // Reset average value       
       for(int i=1;i<_bufSize-1;i++) phData.avgValue+=phData.buf[i];      // Drop highest and lowest value, add the rest together
       
       float pHVol=(float)phData.avgValue*5.0/1024/8;              // average and convert to millivolts
       float phValue = -5.70 * pHVol + 21.34;               // convert millivolts to PH reading
       
       Serial.print("PH = ");
       Serial.println(phValue);
      
       phData.prevPhTime = millis();                              // Reset the timer and buffer index
       phData.phIndex = 0;
    }
  }
}



void PH::setTargetPH(float newTarget) {
  phData.targetPh = newTarget;
}

float PH::getTargetPH() {
  return phData.targetPh;
}

void PH::setPHDelay(unsigned long newDelay) {
  phData.checkPhDelay = newDelay;
}

// Returns the delay time between PH Checks
unsigned long PH::getPHDelay() {
  return phData.checkPhDelay;
}


// Returns the most recent PH reading
float PH::getCurrentPH() {
  return phData.currentPh;
}

// Enter probe calibration mode... 
// TODO: LOTS!!!!
void PH::calibratePH() {
     Serial.println(F("PH Calibration Mode"));

     
     Serial.println(F("Enter Calibration Target 1"));
     Serial.println(F("Waiting to stabilize..."));
     // Timer goes here
     Serial.println(F("Reading Actual 1"));

     
     Serial.println(F("Enter Calibration Target 2"));
     Serial.println(F("Waiting to stabilize..."));
     // Timer goes here
     Serial.println(F("Reading Actual 2"));

     setCalPoints(phData.calActual);
}

void PH::setCalPoints(float calActual[]) {
  for (byte i = 0; i < 2; i++) {
    phData.calActual[i] = calActual[i];
  }
}
