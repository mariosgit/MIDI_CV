/*
 * MIDI_CV.ino
 *
 * Created: 4/3/2017
 * Author: Mario Becker
 * OneNote Bastel/Midi-CV Conv
 *
 * Todo
 * - switching behaviour, Gate when new note?
 * - add midi In/Through
 *
 * Done
 * - works with ES2 (poly=mono2)
 * - multiple DACs ? toggle with 4051
 *
 *
 * hardware = Pololu A-Star -> use Arduino Micro Settings
 */

#include <Arduino.h>
#include <Wire.h>
#include <Timer.h>

#include "mbmidimatrix.h"
#include "mbmidicv.h"
#include "mbinput.h"
#include "vfd.h"
#include "mblog.h"

const int PIN_DISP_RS = 4;
const int PIN_DISP_CS = 5;
const int PIN_LED = 13;

const int PIN_LED_COMMON = 7; // common pin for multiplexed LEDs
const int PIN_LED6 = 8;
const int PIN_LED5 = 9;

Timer _timer;

bool runDemo = true;
int runDemoPos = 0;
byte runDemoVel = 0;
char runDemoNotes[] = {0, 12, 24, 12};

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
/*
void noteOn(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
    midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
}
*/

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
#ifdef _midiusb_
void controlChange(byte channel, byte control, byte value)
{
    midiEventPacket_t event = {0x0B, channel | 0xB0, control, value};
    MidiUSB.sendMIDI(event);
}
#endif

void setup()
{
    randomSeed(analogRead(0));
    Wire.begin(); // join i2c bus (address optional for master)
    Wire.setClock(100000);
    Serial.begin(115200);
    SPI.begin();

    MBCV.begin();
    MIRO.begin();

    VFD.begin(PIN_DISP_CS, PIN_DISP_RS);

    _config.restore(); // reload setup // uncomment for first run !

    MIDI.setHandleClock(MbMidiRouter::handleClock);
    MIDI.setHandleNoteOn(MbMidiRouter::handleNoteOn);
    MIDI.setHandleNoteOff(MbMidiRouter::handleNoteOff);
    // MIDI.setHandleControlChange(MbMidiRouter::handleControlChange);
    //MIDI.setHandleAfterTouchChannel(void (*fptr)(byte channel, byte pressure));
    //MIDI.setHandlePitchBend(void (*fptr)(byte channel, int bend));
    // MIDI.turnThruOn(MIDI_NAMESPACE::Full);
    MIDI.turnThruOn(MIDI_NAMESPACE::DifferentChannel);
    MIDI.begin(_config.myChannel);

    _timer.every(1, InputEvents::updateEncoderCB); // 1ms rotary encoder updates
    _timer.every(1, midiLoop);                     // 1ms rotary encoder updates
    _timer.every(1, MbMidiUSB::loop);              // 1ms rotary encoder updates
    _timer.every(20, MbVFD::loopVFD);
    _timer.every(20, loopLED);
    _timer.every(20, loopVisualization);
    //_timer.every(200, loopVFDVertical);

    //_timer.every(20, ledrefresher); // as fast or as soon as the CV updates ? CV should toggle the LEDs off, the refresh back on
    //_timer.every(100, loopTest);

    //midiin
    //clock interpolator / generator
    //lfo generator

    digitalWrite(PIN_LED_COMMON, LOW);
    pinMode(PIN_LED_COMMON, OUTPUT);
    pinMode(PIN_LED6, OUTPUT);
    pinMode(PIN_LED5, OUTPUT);

    digitalWrite(PIN_LED5, HIGH);
    digitalWrite(PIN_LED6, HIGH);
    // low high -> all off
    // high high -> all off
    // low low -> 5 on
    // high low -> all on
    // LED 6 andersrum einloeten, dann gehts vielleicht
}

byte demoCount = 0;
bool demo = false;
byte demoNotes[16] = {0, 255, 4, 255, 7, 255, 254, 254, 12, 16, 19, 24, 255, 255, 12, 255};
byte demoPos = 0;
byte posTune = 0;
char posTuneNote[6][3] = {"e0", "a0", "d1", "g1", "b1", "e2"};
byte posTunePitch[6] = {4, 9, 14, 19, 23, 28};

// show clock and last note on disp
void loopVisualization()
{
    switch (VFD.getPageNumber())
    {
        case MbVFD::MIDI_OUT:
        {
            if (_inputEvents.isDoubleClicked())
                posTune = (posTune + 1) % 6;

            MBCV.dacCal = (MBCV.dacCal + _inputEvents.getEncVal());

            VFD.position(5, 0);
            VFD.write_page(MbVFD::MIDI_OUT, posTuneNote[posTune]);
            MBCV.updatePitch(posTunePitch[posTune], 1); //E

            String sval1(MBCV.dacCal);
            VFD.position(8, 0);
            VFD.write_page(MbVFD::MIDI_OUT, posTune == 0 ? "synf" : sval1.c_str());
        }
        break;

        case MbVFD::INFO:
        {
            if (_inputEvents.isDoubleClicked())
                demo = !demo;
            if (demo)
            {
                demoCount = (demoCount + 1) % 16;
                if (demoCount == 0)
                {
                    demoPos = (demoPos + 1) % 16;
                    switch (demoNotes[demoPos])
                    {
                    case 0xff:
                        break;
                    case 0xfe:
                        MIRO.handleNoteOff(1, demoNotes[demoPos] + 36, 127);
                        break;
                    default:
                        MIRO.handleNoteOn(1, demoNotes[demoPos] + 36, 127);
                        break;
                    }
                }
                VFD.write_page(MbVFD::INFO, "   -demo-   ");
            }
            else
            {
                VFD.write_page(MbVFD::INFO, "  mb'knobs    ");
            }
        }
        break;
    }
    //VFD.position(11, 0);
    // ableton: if((MIRO.getClockState() % (12*4)) <= 6*4)
    if ((MIRO.getClockState() % (12 * 4)) <= 6 * 4) // 0-191 = 24*4*2 2 takte ala 4ter
    {
        //VFD.write_page(0, "<"); // 7
        VFD.getPage(VFD.INFO)[11] = 11;
        VFD.define_char(11, 0x4487);
    }
    else
    {
        //VFD.write_page(0, ">"); // L
        VFD.getPage(VFD.INFO)[11] = 11;
        VFD.define_char(11, 0x4478);
    }

    VFD.position(0, 0);
    if (MIRO.getLastPitch() != 0xff)
    {
        // note on
        byte note = MIRO.getLastPitch() % 12;
        switch (note)
        {
        case 0:
            VFD.write_page(0, "C");
            break;
        case 1:
            VFD.write_page(0, "C+");
            break;
        case 2:
            VFD.write_page(0, "D");
            break;
        case 3:
            VFD.write_page(0, "D+");
            break;
        case 4:
            VFD.write_page(0, "E");
            break;
        case 5:
            VFD.write_page(0, "F");
            break;
        case 6:
            VFD.write_page(0, "F+");
            break;
        case 7:
            VFD.write_page(0, "G");
            break;
        case 8:
            VFD.write_page(0, "G+");
            break;
        case 9:
            VFD.write_page(0, "A");
            break;
        case 10:
            VFD.write_page(0, "A+");
            break;
        case 11:
            VFD.write_page(0, "B");
            break;
        }
    }
    else
    {
        // note off
        VFD.position(0, 0);
        VFD.write_page(0, "  "); // 7
    }
}

uint16_t loopVisCount = 1;
void loopVFDVertical()
{
    loopVisCount = loopVisCount << 1;
    (LOG << " cnt:").println(loopVisCount, HEX);
    VFD.getPage(VFD.INFO)[1] = 1;
    //VFD.getPage(VFD.INFO)[3] = 2;
    VFD.define_char(1, loopVisCount & 0x00ff);
    //VFD.define_char(2, loopVisCount&0xff00);
    // VFD.define_char(2, 0x043f); //M
    // VFD.define_char(1, 0x4183); //b
    // VFD.define_char(1, 0x2903); //K
    if (loopVisCount >= 0x8000)
        loopVisCount = 0x101;
}

uint8_t ledpos = 0;
void loopLED()
{
    byte pos = VFD.getPageNumber();
    if (pos > 2)
    {
        const int PIN_SELB = 11; //4052 selection pins
        const int PIN_SELA = 12;
        // map VFD page -> LED mux
        byte adrLED[7] = {0, 0, 0, 2, 1, 0, 3};
        digitalWrite(PIN_SELA, adrLED[pos] & 0x1);
        digitalWrite(PIN_SELB, adrLED[pos] & 0x2);
        // enable LED
        digitalWrite(PIN_LED_COMMON, HIGH);
        //LOG << "loopLED mux:" <<adrLED[pos] <<"\n";
    }
    else
    {
        digitalWrite(PIN_LED_COMMON, LOW);
        switch (pos)
        {
        case 4:
            digitalWrite(PIN_LED5, HIGH);
            digitalWrite(PIN_LED6, LOW);
            break;
        default:
            digitalWrite(PIN_LED5, HIGH);
            digitalWrite(PIN_LED6, HIGH);
            break;
        }
        //LOG << "loopLED mux off\n";
    }
    ledpos = (ledpos + 1) % 4;
}

bool bStatus = 0;
uint8_t dacval = 0;
byte bClockStatus = 0;

void midiLoop()
{
    byte maxloop = 10;
    while (MIDI.read()) // && maxloop--) // Is there a MIDI message incoming ?
    {
        //        if(MIDI.getType() == midi::NoteOn)
        //                LOG <<"x" <<_config.myChannel <<"\n";
        if (MIDI.getChannel() == _config.myChannel)
        {
            // Visualization stuff, move to loopVisualization
            switch (MIDI.getType()) // Get the type of the message we caught
            {
            case midi::NoteOn: // If it is a Note,
              break;
            case midi::ProgramChange: // If it is a Program Change,
                break;
            case midi::ControlChange:
                {
                MIRO.handleControlChange(MIDI.getChannel(), MIDI.getData1(), MIDI.getData1() );

                String sval1(MIDI.getData1());
                VFD.position(9, 0);
                VFD.write_page(3, "   ");
                VFD.position(9, 0);
                VFD.write_page(3, sval1.c_str());
                LOG << "CC " <<MIDI.getData1() <<" " <<MIDI.getData1() <<"\n";
                }
                break;
            // See the online reference for other message types
            default:
                LOG.print(MIDI.getType(), HEX);
                break;
            }
        }

        // soft through, turned through on in setup!
        // MIDI.send(MIDI.getType(), MIDI.getData1(), MIDI.getData2(), MIDI.getChannel());
    }
}

void loop()
{
    _timer.update();
}
