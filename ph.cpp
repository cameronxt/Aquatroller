#include "ph.h"

PH::PH() {
  
}

void PH::init() {
  
}

void PH::loop(unsigned long currentTime){
  if (prevPhTime - currentTime >= checkPhDelay) {
    //Serial.println(F("its time to check it"));
    buf[phIndex]=analogRead(pin);               // store anolg value into buffer position
    currentTime = millis();                        // Reset timer to allow ph sensor to stabilize
    if (phIndex < bufSize){
      phIndex++;
    } else {
      for(int i=0;i<(bufSize-1);i++) {
        for(int j=i+1;j<bufSize;j++) {
          if(buf[i]>buf[j]) {
            temp=buf[i];
            buf[i]=buf[j];
            buf[j]=temp;
          }
        }
      }
       avgValue=0;
       for(int i=2;i<8;i++) avgValue+=buf[i];
       float pHVol=(float)avgValue*5.0/1024/6;
       float phValue = -5.70 * pHVol + 21.34;
       Serial.print("PH = ");
       Serial.println(phValue);
       currentTime = millis();
       phIndex = 0;
    }
  }
}
