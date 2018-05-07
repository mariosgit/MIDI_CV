#pragma once

// VFD
// protocol for using a Samsung HCS12SS59T VFD (from Pollin.de)
// function names like in the DOGM_7036 lib
//
// Use write_page function to place infos on one of the menu pages.

#include <SPI.h>
#include <Arduino.h>

#include "mbinput.h"

class MbVFD
{
public:
    MbVFD();

    enum pageNames {
        INFO = 0,
        MIDI_IN = 1,
        MIDI_OUT = 2,
        CV0 = 3,
        CV1 = 4,
        CV2 = 5,
        CV3 = 6,
    };

    void begin(byte pinCS, byte pinRS = 0);
    void test();

    void clear_display();
    void position(byte x, byte y);
    void write_page(byte page, const char*);
    char* getPage(byte page) { return _page[page]; }

    void define_char(byte adr, uint16_t ch);
    //    DOG.displ_onoff(true);      // turn Display on
    //    DOG.cursor_onoff(false);    // turn Curosor blinking on

    static void loopVFD();

    inline byte getPageNumber() { return _loopVFD;}

    private:
    void string(const char*);

    byte _pinCS, _pinRS;
    SPISettings _spiDispSettings;
    byte _testStep;
    byte _posX;
    bool _vertical;

    byte _chars[128];

    const static byte _maxPages = 7;
    char _page[_maxPages][16];

    // loop vars
    static byte _loopVFD;
    static byte _loopVFDi;
};

extern MbVFD VFD;
