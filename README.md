# mb'midiCV

in development ! Check later...

System software for my MIDI to CV interface, which can be found there: ADD LINK.

To keep a bit of cleanyness, I split the code into the main ino and some potential
lib files (.h + .cpp).
The latter could also be used as a stand alone lib when put into your Arduino
library folder.

### How it works
Well uhm yeah it does somehow...

### Display
I used a VFD Samsung HCS12ss59t available at [pollin.de](https://www.pollin.de/p/vakuum-fluoreszenzdisplay-samsung-hcs-12ss59t-12x1-121466). However it is somewhat optional or replaceable by DOGM or some other SPI/I2C based displays.

### Used Libraries
- Timer
- MIDI
- MidiUSB
- encoder

