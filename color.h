#ifndef COLOR_H
#define COLOR_H

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
