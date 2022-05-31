#ifndef PALETTES_H
#define PALETTES_H

#include <FastLED.h>

const TProgmemRGBPalette16 *palettes[] = {
    &CloudColors_p,
    &LavaColors_p,
    &OceanColors_p,
    &ForestColors_p,
    &RainbowColors_p,
    &RainbowStripeColors_p,
    &PartyColors_p,
    &HeatColors_p
};
const uint8_t palette_count = sizeof(palettes) / sizeof(TProgmemRGBGradientPalettePtr);

#endif
