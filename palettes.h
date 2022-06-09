#ifndef PALETTES_H
#define PALETTES_H

#include <FastLED.h>

extern const TProgmemRGBPalette16* palettes[] = {
    &CloudColors_p,
    &LavaColors_p,
    &OceanColors_p,
    &ForestColors_p,
    &RainbowColors_p,
    &RainbowStripeColors_p,
    &PartyColors_p,
    &HeatColors_p
};
#define PALETTE_COUNT sizeof(palettes) / sizeof(TProgmemRGBGradientPalettePtr)

#endif
