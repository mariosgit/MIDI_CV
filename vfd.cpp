#include <MIDI.h>
#include "mblog.h"
#include "mbmidimatrix.h"
#include "vfd.h"

byte MbVFD::_loopVFD  = 0;
byte MbVFD::_loopVFDi = 0;

MbVFD::MbVFD() :
  _spiDispSettings(2000000, LSBFIRST, SPI_MODE3), // mode 0 or 3 clock in on rising edge 0 = 1=inactive mode 3=0-inactive
  _testStep(0),
  _posX(0),
  _vertical(false)
{
    for(byte i = 0; i < 128; i++)
      _chars[i] = 0x3a;
    for(byte i = 0; i < 16; i++)
        _chars[i] = i;

    _chars['\\'] = 0x2c;
    _chars[')'] = 0x2d;
    _chars['^'] = 0x2e;
    _chars['_'] = 0x2f;

    _chars[' '] = 0x30;
    _chars[':'] = 0x30;
    _chars['"'] = 0x32;
    _chars['$'] = 0x34;
    _chars['\''] = 0x37;
    _chars['<'] = 0x38;
    _chars['>'] = 0x39;
    _chars['*'] = 0x3a;
    _chars['+'] = 0x3b;
    _chars[' '] = 0x3c;
    _chars['-'] = 0x3d;
    _chars['/'] = 0x3f;

    _chars['!'] = 0x41;

    for(byte i = 0; i < 26; i++)
        _chars['A'+i] = 0x11+i;
    for(byte i = 0; i < 26; i++)
        _chars['a'+i] = 0x11+i;
    for(byte i = 0; i < 10; i++)
        _chars['0'+i] = 0x40+i;

    write_page(INFO,     "  mb'knobs     ");
    write_page(MIDI_IN,  "Channel:       ");
    write_page(MIDI_OUT, "Tune           "); //Midi out       ");
    write_page(CV0,      "CV0 gate       ");
    write_page(CV1,      "CV1 pitch      ");
    write_page(CV2,      "CV2 velocity   ");
    write_page(CV3,      "CV3 CC  xxx yyy");
}

void MbVFD::begin(byte pinCS, byte pinRS) {
  _pinCS = pinCS;
  _pinRS = pinRS;

  SPI.begin();

  pinMode(_pinCS, OUTPUT);
  digitalWrite(_pinCS, HIGH);

  if(_pinRS) {
    pinMode(_pinRS, OUTPUT);
    digitalWrite(_pinRS, LOW);
    delay(50);
    digitalWrite(_pinRS, HIGH);
  }

  // init sequence
  digitalWrite(_pinCS, LOW);
  SPI.beginTransaction(_spiDispSettings);
      SPI.transfer(0x6C); //12 digets
  SPI.endTransaction();
  digitalWrite(_pinCS, HIGH);

  digitalWrite(_pinCS, LOW);
  SPI.beginTransaction(_spiDispSettings);
      SPI.transfer(0x5f); //mid level brightness 0x50|val
  SPI.endTransaction();
  digitalWrite(_pinCS, HIGH);

  digitalWrite(_pinCS, LOW);
  SPI.beginTransaction(_spiDispSettings);
      SPI.transfer(0x70); //lights on
  SPI.endTransaction();
  digitalWrite(_pinCS, HIGH);
}

void MbVFD::clear_display() {
  digitalWrite(_pinCS, LOW);
  SPI.beginTransaction(_spiDispSettings);
  SPI.transfer(0x10); //command data, followed by 16bit values
  SPI.transfer(0x3c); SPI.transfer(0x3c);
  SPI.transfer(0x3c); SPI.transfer(0x3c);
  SPI.transfer(0x3c); SPI.transfer(0x3c);
  SPI.transfer(0x3c); SPI.transfer(0x3c);
  SPI.transfer(0x3c); SPI.transfer(0x3c);
  SPI.transfer(0x3c); SPI.transfer(0x3c);
  SPI.endTransaction();
  digitalWrite(_pinCS, HIGH);
}

void MbVFD::position(byte x, byte y) {
  _posX = x % 16;
  _vertical = (y != 0);
}

void MbVFD::write_page(byte page, const char *msg) {
  byte len = strlen(msg);
  for(int i = _posX; i < _posX + len; i++)
    _page[page][i] = *msg++;
}

void MbVFD::string(const char *msg) {
  digitalWrite(_pinCS, LOW);
  SPI.beginTransaction(_spiDispSettings);

  byte len = strlen(msg);
  byte start = (12-len-_posX)&0xf;
  /*
  LOG <<"str len:";
  LOG <<len;
  LOG <<",";
  LOG <<start;
  LOG <<"\n";
  */
  SPI.transfer(0x10|start); //command data
  for(;len > 0;) {
    SPI.transfer(_chars[msg[--len]]);
  }
  SPI.endTransaction();
  digitalWrite(_pinCS, HIGH);

  _posX = (_posX + len) % 16;
}

void MbVFD::define_char(byte adr, uint16_t ch) {
    digitalWrite(_pinCS, LOW);
    SPI.beginTransaction(_spiDispSettings);
    SPI.transfer(0x20 | adr);// | (adr&0xf)); //command cgram, followed by 16bit values
    SPI.transfer(ch&0xff); SPI.transfer(ch>>8);
    SPI.endTransaction();
    digitalWrite(_pinCS, HIGH);
}

void MbVFD::loopVFD() {

  if(_inputEvents.isClicked()) {
      _loopVFD = (_loopVFD + 1) % _maxPages;
      LOG <<"VFD isClicked menu:" <<_loopVFD <<"\n";
  }

  //VFD.test();
  VFD.clear_display();
  switch(_loopVFD) {
    case INFO: {
      _loopVFDi = (_loopVFDi + _inputEvents.getEncVal()) % 16;
      VFD.position(_loopVFDi, 1);
      VFD.string(VFD._page[_loopVFD]);
    } break;
    case MIDI_IN: {
      auto enc = _inputEvents.getEncVal();
      if(enc)
        _config.myChannel = (enc + _config.myChannel);
      if(_config.myChannel > 16)
        _config.myChannel = 1;
      if(_config.myChannel < 1)
        _config.myChannel = 16;
      MIDI.setInputChannel(_config.myChannel); // use for later change
      VFD.position(0, 0);
      VFD.string(VFD._page[_loopVFD]);
      String sval1(_config.myChannel);
      VFD.position(8, 0);
      VFD.string(sval1.c_str());
    } break;
    case CV3: {
      auto enc = _inputEvents.getEncVal();
      if(enc)
        _config.ccCV3 = (enc + _config.ccCV3);
      if(_config.ccCV3 > 127)
        _config.ccCV3 = 0;
      if(_config.ccCV3 < 0)
        _config.ccCV3 = 127;
      VFD.position(0,0);
      VFD.string(VFD._page[_loopVFD]);
      VFD.position(6, 0);
      VFD.write_page(3, "   ");
      String sval1(_config.ccCV3);
      VFD.position(6, 0);
      VFD.string(sval1.c_str());
    } break;
    default: {
      VFD.position(0,0);
      VFD.string(VFD._page[_loopVFD]);
    } break;
  }
  //_loopVFD = (_loopVFD + 1)%4;
}

MbVFD VFD;
