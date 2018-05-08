#include "mbConfig.h"
#include "mbmidimatrix.h"
#include "EEPROM.h"

//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
midi::MidiInterface<HardwareSerial> MIDI((HardwareSerial&)Serial1);
MbMidiConfig _config;
MbMidiRouter MIRO;
MbMidiUSB MIDIUSB;

void MbMidiConfig::store()
{
    int address = 0;
    EEPROM.update(address++, myChannel);
    EEPROM.update(address++, lowNote);
    EEPROM.update(address++, ccCV3);
}
void MbMidiConfig::restore()
{
    int address = 0;
    myChannel = EEPROM.read(address++);
    lowNote   = EEPROM.read(address++);
    ccCV3     = EEPROM.read(address++);
}

/*************************************************/

int  MbMidiRouter::_bend = 0;
bool MbMidiRouter::_running = false;

MbMidiRouter::MbMidiRouter() :
    _lastPitch(0xff),
    _clockState(false)
{
}

void MbMidiRouter::begin() {

}

void MbMidiRouter::handleNoteOn(byte channel, byte pitch, byte velocity)
{
    if(channel != _config.myChannel)
        return;

    MIRO._lastPitch = pitch;
#ifndef MB_MIDICV_MODE_DOUBLE_GATE_PITCH
    if(pitch < _config.lowNote)
        return;

    LOG <<"handleNoteOn ch:" <<channel <<", pitch:" <<pitch <<", vel:" <<velocity <<"\n";

    digitalWrite(PIN_LED, LOW);
    MBCV.updateCV(0, 0); //gate off
    MBCV.updatePitch(pitch - _config.lowNote, 1);
    MBCV.updateBend(_bend, 1);
    MBCV.updateCV(velocity, 2);
    MBCV.updateCV(0xff, 0); //gate on
    digitalWrite(PIN_LED, HIGH);
#else
    if(pitch < 60) // C4
    {
        digitalWrite(PIN_LED, LOW);
        MBCV.updateCV(0, 0); //gate off
        MBCV.updatePitch(pitch - 0, 1);
        MBCV.updateBend(_bend, 1);
        MBCV.updateCV(0xff, 0); //gate on
        digitalWrite(PIN_LED, HIGH);
    }
    else
    {
        digitalWrite(PIN_LED, LOW);
        MBCV.updateCV(0, 2); //gate off
        MBCV.updatePitch(pitch - 60, 3);
        MBCV.updateBend(_bend, 3);
        MBCV.updateCV(0xff, 2); //gate on
        digitalWrite(PIN_LED, HIGH);
    }
#endif
}

void MbMidiRouter::handleNoteOff(byte channel, byte pitch, byte velocity)
{
    if(channel != _config.myChannel)
        return;
    MIRO._lastPitch = 0xff;
#ifndef MB_MIDICV_MODE_DOUBLE_GATE_PITCH
    digitalWrite(PIN_LED, LOW);
    MBCV.updateCV(0, 0); //gate off
#else
    if(pitch < 60)
    {
        digitalWrite(PIN_LED, LOW);
        MBCV.updateCV(0, 0); //gate off
    }
    else
    {
        digitalWrite(PIN_LED, LOW);
        MBCV.updateCV(0, 2); //gate off
    }
#endif
}

void MbMidiRouter::handlePitchBend(byte channel, int bend)
{
    _bend = bend;
    MBCV.updateBend(_bend, 1);
#ifdef MB_MIDICV_MODE_DOUBLE_GATE_PITCH
    MBCV.updateBend(_bend, 3);
#endif
    // LOG <<"handlePitchBend ch:" <<channel <<", bend:" <<_bend <<"\n";
    // MBCV.updateCV(value, 1);
}

void MbMidiRouter::handleControlChange(byte channel, byte cc, byte value)
{
    if(channel != _config.myChannel)
        return;
    if(cc != _config.ccCV3)
        return;
//   LOG <<"handleControlChange:" <<channel <<", cc:" <<cc <<", value:" <<value <<"\n";
#ifndef MB_MIDICV_MODE_DOUBLE_GATE_PITCH
    MBCV.updateCV(value, 3);
#endif
}

void MbMidiRouter::handleClock()
{
    MIRO._clockState = (MIRO._clockState + 1) % (24*8);
    // LOG <<MIRO._clockState <<",";
}

void MbMidiRouter::handleStart()
{
    MIRO._clockState = 0;
    MIRO._running = true;
}

void MbMidiRouter::handleStop()
{
    // MIRO._clockState = 0;
    MIRO._running = false;

    MBCV.updateBend(0, 0);
    MBCV.updateBend(0, 1);
    MBCV.updateBend(0, 2);
    MBCV.updateBend(0, 3);
    MBCV.updateCV(0, 0);
    MBCV.updateCV(0, 1);
    MBCV.updateCV(0, 2);
    MBCV.updateCV(0, 3);
}

#define _midiusb_ 1
#ifdef _midiusb_
#include "MIDIUSB.h"
#endif

void MbMidiUSB::loop()
{
#ifdef _midiusb_
    midiEventPacket_t rx;

    rx = MidiUSB.read();
    if (rx.header != 0)
    {
        //digitalWrite(PIN_LED, bStatus);
        int note = rx.byte2 - 48;
        if(rx.header == 0x09)
        {
            if(note >= 0)
            {
              MbMidiRouter::handleNoteOn(rx.header & 0xf0, rx.byte2, rx.byte3);
            }
        }
        else if(rx.header == 0x08)
        {
          MbMidiRouter::handleNoteOff(rx.header & 0xf0, rx.byte2, rx.byte3);
        }
        else if(rx.byte1 == 0xF8)
        {
            MbMidiRouter::handleClock();
        }

        // (LOG <<"USB got: ").print(rx.header, HEX);
        // (LOG <<",").print(rx.byte1, HEX);
        // (LOG <<",").print(rx.byte2, HEX);
        // (LOG <<",").println(rx.byte3, HEX);
    }
#endif
}
