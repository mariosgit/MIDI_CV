/*
* class MbMidiCV
* Controls the DAC output
* 
*/

#include <Wire.h>

#include "mblog.h"
#include "mbmidicv.h"

MbMidiCV::MbMidiCV()
{
    //dacCal = 7500; //4,46VV
    dacCal = 6791; //5V 
}

void MbMidiCV::begin() {
    pinMode(PIN_SELA,  OUTPUT);
    pinMode(PIN_SELB,  OUTPUT);
}

void MbMidiCV::updatePitch(byte key, byte adr)
{
    //burn some time, 4052 needs setup time of 200us
    //for(byte i = 0; i < 10; i++)
    {
        digitalWrite(PIN_SELA, adrDAC[adr]&0x1);
        digitalWrite(PIN_SELB, adrDAC[adr]&0x2);
    }

    uint32_t val = ((key * dacCal)/100ul);
    if(key == 0xff)
        val = 0xfff; //max val for gate on off
    buffer[adr] = val;

    Wire.beginTransmission(0x60);
    Wire.write(byte((val & 0x0f00) >> 8));
    Wire.write(byte(val & 0xff));
    Wire.endTransmission();
    /*
    LOG <<(adr) <<".";
    LOG <<(adrDAC[adr]) <<".";
    //LOG <<(adrDAC[adr]&0x1) <<".";
    //LOG <<(adrDAC[adr]&0x2);
    LOG <<" updateCV ch:";
    LOG <<adr;
    LOG <<", key:";
    LOG <<key;
    LOG <<", val:";
    LOG <<val;
    LOG <<"\n";
    */
}

void MbMidiCV::updateBend(int bend, byte adr)
{
    /// range is -8192..8191 !!!

    //burn some time, 4052 needs setup time of 200us
    //for(byte i = 0; i < 10; i++)
    {
        digitalWrite(PIN_SELA, adrDAC[adr]&0x1);
        digitalWrite(PIN_SELB, adrDAC[adr]&0x2);
    }

    int32_t iDacCal = dacCal;
    int32_t maxbend = ((bend*(12 * iDacCal))/819200);

    int32_t val = buffer[adr];
    val += maxbend;
    if(val < 0)
        val = 0;
    if(val > 4095)
        val = 4095;

    // LOG <<"bend: note:";
    // LOG <<static_cast<uint32_t>(buffer[adr]);
    // LOG <<" bend:";
    // LOG <<bend;
    // LOG <<" maxbend:";
    // LOG <<maxbend;
    // LOG <<" val:";
    // LOG <<val;
    // LOG << "\n";

    Wire.beginTransmission(0x60);
    Wire.write(byte((val & 0x0f00) >> 8));
    Wire.write(byte(val & 0xff));
    Wire.endTransmission();
}

void MbMidiCV::updateCV(byte value, byte adr)
{
    //burn some time, 4052 needs setup time of 200us
    //for(byte i = 0; i < 10; i++)
    {
        digitalWrite(PIN_SELA, adrDAC[adr]&0x1);
        digitalWrite(PIN_SELB, adrDAC[adr]&0x2);
    }

    uint32_t val = value<<5;

    Wire.beginTransmission(0x60);
    Wire.write(byte((val & 0x0f00) >> 8));
    Wire.write(byte(val & 0xff));
    Wire.endTransmission();
}

MbMidiCV MBCV; // one global instance
