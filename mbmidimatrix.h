/* take input from different midi devices (usb/serial/serial1...) and
 * - route them to another output/channel
 * - filter messages
 *
 * Implementation
 * - have a #In x #Out array
 * - on each chunktion a Filter/Mapper object
 *   - Filter - removes message types
 *   - Mapper - defines the channels to receive and send
 * - An input mapper/listener must quickly deside to which outputs something is send
 * - Then comes the Filter per Output
 * - Then commes the mapper per Output
 *
 */

#pragma once

#include <Arduino.h>
#include <MIDI.h>
#include "mblog.h"
#include "mbmidicv.h"

struct MbMidiConfig
{
    byte lowNote = 36;  // electribes 2. octave button
    byte myChannel = 1; // textual 1-16
    byte ccCV3 = 74;
    void store();
    void restore();
};

extern MbMidiConfig _config;

class MbMidiRouter
{
  public:
    MbMidiRouter();
    void begin();
    // callbacks for serial midi
    static void handleNoteOn(byte channel, byte pitch, byte velocity);
    static void handleNoteOff(byte channel, byte pitch, byte velocity);
    static void handleControlChange(byte channel, byte pitch, byte velocity);
    static void handleClock();

    inline byte getLastPitch() { return _lastPitch; }
    inline byte getClockState() { return _clockState; }
  private:
    static const int PIN_LED = 13;

    // status for visualization
    byte _lastPitch;
    byte _clockState;
};
extern MbMidiRouter MIRO;

// wrapper for MIDIUSB lib, calls MbMidiRouter callbacks like the MIDI lib
class MbMidiUSB
{
  public:
    static void loop();

  private:
};
extern MbMidiUSB MIDIUSB;

//extern MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
extern midi::MidiInterface<HardwareSerial> MIDI;
