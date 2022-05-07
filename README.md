# Arduino MIDI to ARGB/RGBW/DMX
Convert Midi input to ARGB, RGBW, and DMX outputs with an Arduino.

## Hardware
This project is designed specifically for the [Adafruit Itsy Bitsy 32u4 - 5V 16MHz](https://learn.adafruit.com/introducting-itsy-bitsy-32u4/), but any other Arduino-compatible MCU with similar specifications should be usable with minor modifications. The PCB design will require modification to use a different development board.

For 5V/12V RGBW operation, you can choose between NPN or N-channel MOSFETs in a TO-220 package _(ie: IRF540N, IRLB8721, or TIP-120)_. The resistors before the gate may not be necessary if you use a MOSFET, but are necessary if you use an NPN transistor. If you are using 5V LEDs, you'll need to bridge JP1 and make sure not to also plug a 12V source into the barrel jack at the same time or you may fry the arduino and any devices plugged into the ARGB connectors. If you use 12V LEDs, make sure that JP1 is not connected and provide a separate 12V power source into the barrel jack that is center-positive.

An XLR connector is provided as an alternative to the ARGB pin header. This connector is non-standard and is intended for custom installations where long, secure cable runs are desired. Avoid mixing the DMX and ARGB connections, though I've attempted to make their pinouts very similar to avoid shorting.

## MIDI Implementation

> The software has not yet been finalized. MIDI messages may be re-organized to provide improve accessibility. See [arduino-midi-argb-dmx.ino](arduino-midi-argb-dmx.ino) for details.

## Software Dependencies
* [Arduino MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library/)
* [FastLED](https://fastled.io/)
* [DmxSimple](https://github.com/PaulStoffregen/DmxSimple)
