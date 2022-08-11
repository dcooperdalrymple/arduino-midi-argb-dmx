#ifndef PALETTES_H
#define PALETTES_H

#include "color_utils.h"

extern const TProgmemRGBPalette16 CloudColors_p PROGMEM; /// Cloudy color pallete
extern const TProgmemRGBPalette16 LavaColors_p PROGMEM; /// Lava colors
extern const TProgmemRGBPalette16 OceanColors_p PROGMEM; /// Ocean colors, blues and whites
extern const TProgmemRGBPalette16 ForestColors_p PROGMEM; /// Forest colors, greens
extern const TProgmemRGBPalette16 RainbowColors_p PROGMEM; /// HSV Rainbow
#define RainbowStripesColors_p RainbowStripeColors_p
extern const TProgmemRGBPalette16 RainbowStripeColors_p PROGMEM; /// HSV Rainbow colors with alternatating stripes of black

/// HSV color ramp: blue purple ping red orange yellow (and back)
/// Basically, everything but the greens, which tend to make
/// people's skin look unhealthy.  This palette is good for
/// lighting at a club or party, where it'll be shining on people.
extern const TProgmemRGBPalette16 PartyColors_p PROGMEM;

/// Approximate "black body radiation" palette, akin to
/// the FastLED 'HeatColor' function.
/// Recommend that you use values 0-240 rather than
/// the usual 0-255, as the last 15 colors will be
/// 'wrapping around' from the hot end to the cold end,
/// which looks wrong.
extern const TProgmemRGBPalette16 HeatColors_p PROGMEM;

#define PALETTE_COUNT 8
extern const TProgmemRGBPalette16* GlobalPalettes[PALETTE_COUNT];
//#define PALETTE_COUNT sizeof(GlobalPalettes) / sizeof(TProgmemRGBGradientPalettePtr)

#endif
