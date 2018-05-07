#pragma once

#include "Arduino.h"

class InputEvents {

public:
  int16_t getEncVal() {
    int16_t result = encOff;
    encOff = 0;
    return result;
  }
  bool isClicked() {
    bool result = encClicked;
    encClicked = false;
    return result;
  }
  bool isDoubleClicked() {
    bool result = encDoubleClicked;
    encDoubleClicked = false;
    return result;
  }

  static void updateEncoderCB();
  
private:
  volatile int16_t encOff = 0;
  volatile bool encClicked = false;
  volatile bool encDoubleClicked = false;
};

extern InputEvents _inputEvents;

