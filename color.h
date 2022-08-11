#ifndef COLOR_H
#define COLOR_H

#include "color_sysdefs.h"
#include "lib8tion.h"
#include "progmem.h"
#include "color_pixeltypes.h"
#include "color_hsv2rgb.h"
#include "color_utils.h"

enum ColorMode : uint8_t {
    ColorOff,
    SolidRgb,
    SolidHsv,
    Palette,
    ChaseRgb,
    ChaseHsv
};
#define COLOR_MODES 6

#endif
