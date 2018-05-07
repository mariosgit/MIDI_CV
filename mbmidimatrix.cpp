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
    if(pitch < _config.lowNote)
        return;

    LOG <<"handleNoteOn ch:" <<channel <<", pitch:" <<pitch <<", vel:" <<velocity <<"\n";

    digitalWrite(PIN_LED, LOW);
    MBCV.updateCV(0, 0); //gate off
    MBCV.updatePitch(pitch - _config.lowNote, 1);
    MBCV.updateCV(velocity, 2);
    MBCV.updateCV(0xff, 0); //gate on
    digitalWrite(PIN_LED, HIGH);
}

void MbMidiRouter::handleNoteOff(byte channel, byte pitch, byte velocity)
{
    if(channel != _config.myChannel)
        return;
    MIRO._lastPitch = 0xff;
    digitalWrite(PIN_LED, LOW);
    MBCV.updateCV(0, 0); //gate off
}

void MbMidiRouter::handleControlChange(byte channel, byte cc, byte value)
{
  if(channel != _config.myChannel)
    return;
  if(cc != _config.ccCV3)
    return;
//   LOG <<"handleControlChange:" <<channel <<", cc:" <<cc <<", value:" <<value <<"\n";
  MBCV.updateCV(value, 3);
}

void MbMidiRouter::handleClock()
{
    MIRO._clockState = (MIRO._clockState + 1) % (24*8);
    // LOG <<MIRO._clockState <<",";
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
