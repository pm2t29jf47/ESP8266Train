#include "ThrottleWrapper.h"
#include "Arduino.h"

void ThrottleWrapper::applyThrottle(int percent) {
  switch (percent) {
    case 10:
      _pwmValue = 100;
      _chimneyLowHighPeriod = 1000;
      break;
    case 20:
      _pwmValue = 120;
      _chimneyLowHighPeriod = 900;
      break;
    case 30:
      _pwmValue = 135;
      _chimneyLowHighPeriod = 800;
      break;
    case 40:
      _pwmValue = 155;
      _chimneyLowHighPeriod = 700;
      break;
    case 50:
      _pwmValue = 170;
      _chimneyLowHighPeriod = 600;
      break;
    case 60:
      _pwmValue = 185;
      _chimneyLowHighPeriod = 500;
      break;
    case 70:
      _pwmValue = 200;
      _chimneyLowHighPeriod = 400;
      break;
    case 80:
      _pwmValue = 215;
      _chimneyLowHighPeriod = 300;
      break;
    case 90:
      _pwmValue = 230;
      _chimneyLowHighPeriod = 200;
      break;
    case 100:
      _pwmValue = 255;
      _chimneyLowHighPeriod = 100;
      break;
    default:
      _pwmValue = 0;
      _chimneyLowHighPeriod = 2000;
      break;
  }
}

void ThrottleWrapper::initializeChimney(int chimneyPin) {
  _chimneyPin = chimneyPin;
  pinMode(_chimneyPin, OUTPUT);
}

void ThrottleWrapper::handleChimney() {
  if (millis() - _chimneyTimer >= _chimneyLowHighPeriod) {
    _chimneyTimer = millis();

    if (_isChimneyLow) {
      analogWrite(_chimneyPin, 255);
    } else {
      analogWrite(_chimneyPin, 100);
    }

    _isChimneyLow = !_isChimneyLow;
  }
}

void ThrottleWrapper::initializeEngine(int enginePin) {
  _enginePin = enginePin;
  pinMode(_enginePin, OUTPUT);
}

void ThrottleWrapper::handleEngine() {
  analogWrite(_enginePin, _pwmValue);
}