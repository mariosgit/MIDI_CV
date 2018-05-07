#include "mbinput.h"

#include "mblog.h"

#include <ClickEncoder.h>
ClickEncoder _encoder(A0, A1, A10, 2, LOW);  // A B Button, ticksPerStep

void InputEvents::updateEncoderCB()
{
    _encoder.service();

    auto button = _encoder.getButton();
    
    if(button == ClickEncoder::Clicked)
    {
        _inputEvents.encClicked = true;
        LOG << "clicked\n";
    }
    if(button == ClickEncoder::DoubleClicked)
    {
        _inputEvents.encDoubleClicked = true;
        LOG << "double clicked\n";
    }
    
    auto val = _encoder.getValue();
    if(val != 0)
    {
        _inputEvents.encOff -= val; // reverse direction
    }
}

InputEvents _inputEvents;

