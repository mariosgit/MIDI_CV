#pragma once

#include <Arduino.h>

/* class MbMidiCV
  This controls the CV output DACs.
  The hardware has 4x MCP4726. 
  The I2C clock line is multiplexed with a 4052 (PIN_SEL[AB]).
  The other half of the 4052 controls the LEDs.
*/

class MbMidiCV
{
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
