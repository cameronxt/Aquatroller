#include "lights.h"

Light::Light(Adafruit_PWMServoDriver *pwm, RTC_DS3231 *rtc) {
  _pwm = pwm;
  _rtc = rtc;
  _data.normalFadeDelay = 5;
  _data.testFadeDelay = 5;
  //  setSyncProvider(_rtc->get);
  // _targetBright = _data.normalMaxBright;
}

void Light::init() {
  Wire.setClock(400000);
  _pwm->begin();
  // pwm.setOutputMode(false);           // Set PWM Driver to open Drain for LEDs
  _pwm->setPWMFreq(1000);                // Set PWM Hz to reduce buzzing sound and Meanwell LLD's have a max of 1kHz
  for (int i = 0; i < 6; i++) {
    _pwm->setPin(i, 0, 0); // turn all channels off
  }
  _data.normalMaxBright = 4096 * .55;
}

// Main loop function.
// Calls light control based on time and
//  _data.mode
void Light::loop(unsigned long ssm) {
  //_rtc->now();
  switch (_data.mode) {
    case NORMAL:
      normalLights(ssm);
      break;
    case SUNLIGHT:
      break;
    case CUSTOM:
      break;
    case TEST:
      testLights(ssm);
      break;

  }
}


bool Light::isLightTime(unsigned long ssm) {
  //  Serial.print ("Current ");
  //  Serial.println (ssm);
  //  Serial.print ("onTime ");
  //  Serial.println (_data.onTime);
  //  Serial.print ("OffTime ");
  //  Serial.println (_data.offTime);

  if ((ssm >= this->_data.onTime) && (ssm < _data.offTime)) {
    return true;
  } else {
    return false;
  }

}

void Light::normalLights(unsigned long ssm) {
  static unsigned long prevMillis;

  if (millis() - prevMillis > _data.normalFadeDelay) {     // Wait for delay
    //Serial.println("Light Delay Exceeded");
    if (isLightTime(ssm)) {                           // if light is supposed to be on
      _targetBright = _data.normalMaxBright;
      if ((_targetBright > _normalCurrentBright) && (_normalCurrentBright < _data.normalMaxBright)) {             // and target bright is brighter than current

        _normalCurrentBright++;                                 // make one step brighter
        for (int i = 0; i < 6; i++) {                           // set all cahnnels to new value
          _pwm->setPin(i, _normalCurrentBright, 0);
        }
      }
    } else {                                                    // If light is supposed to be off
      _targetBright = 0;
      if ((_targetBright < _normalCurrentBright) && (_normalCurrentBright > 0)) {             // and target brightness is lower than current bright
        _normalCurrentBright--;                                 // make one step dimmer
        for (int i = 0; i < 6; i++) {                           // Set all channels to new value
          _pwm->setPin(i, _normalCurrentBright, 0);
        }
      }
    }

    prevMillis = millis();                                      // reset delay timer
  }
}



// Test function to make sure everything is working right
void Light::testLights(unsigned long ssm) {
  static int currentBright = 0;

  static unsigned long prevFadeTime = millis();
  static bool isRising;
  //Serial.println("------------------------------------------");
  //Serial.print("Current Millis: ");
  //Serial.println(currentMillis);
  //Serial.print("Previous Fade Time: ");
  //Serial.println(prevFadeTime);

  if (millis() - prevFadeTime >= _data.testFadeDelay) {
    //Serial.println("Time to adjust light");
    if (isRising) {
      if (currentBright < 4096) {
        currentBright++;
      } else {
        isRising = false;
      }
    } else {
      if (currentBright > 0) {
        currentBright--;
      } else {
        isRising = true;
      }
    }
    for (int i = 0; i < 6; i++) {
      //Serial.println("Adjusting Light");
      _pwm->setPin(i, currentBright, 0);
    }
    prevFadeTime = millis();
  }
}

void Light::setMode(byte mode) {
  _data.mode = mode;
}

byte Light::getMode() {
  return _data.mode;
}

void Light::setOnTime(unsigned long timeOn) {
  _data.onTime = timeOn;

}
unsigned long Light::getOnTime() {
  return _data.onTime;
}
void Light::setOffTime(unsigned long timeOff) {
  _data.offTime = timeOff;
}
unsigned long Light::getOffTime(byte ch) {
  return _data.offTime;
}
