#ifndef PRESET_H
#define PRESET_H

#include <avr/pgmspace.h>
#include <FastLED.h>
#include "color.h"

typedef struct _PresetData {
    uint8_t note;
    ColorMode mode;
    uint8_t palette;
    uint8_t palette_speed;
    CRGB color_rgb;
    CHSV color_hsv;
} PresetData;

const PROGMEM PresetData default_preset = {
    .note = 0,
    .mode = ColorOff,
    .palette = 0,
    .palette_speed = 32,
    .color_rgb = CRGB::Black,
    .color_hsv = CHSV(0, 0, 0)
};

#endif
