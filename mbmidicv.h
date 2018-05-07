#pragma once

// hardware reletated functions

#include <Arduino.h>

class MbMidiCV {

public:
  MbMidiCV();
  void begin();

  //! Output a note value, this is translated into a voltage
  void updatePitch(byte key, byte adr);

  //! Output a plain value 0-127
  void updateCV(byte value, byte adr);

private:
  const int PIN_SELB = 11; //4052 selection pins
  const int PIN_SELA = 12;

  const uint8_t adrLED[4] = {3,0,1,2};
  const uint8_t adrDAC[4] = {3,0,1,2};
  //const uint8_t adrDAC[4] = {2,1,0,3};

public:
  uint32_t dacCal;
};

extern MbMidiCV MBCV;
