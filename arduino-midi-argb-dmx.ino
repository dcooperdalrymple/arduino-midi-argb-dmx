/**
 * Title:       Arduino MIDI ARGB DMX
 * Created by:  Cooper Dalrymple
 * Date:        2022-05-06
 * License:     GNU GPL v3.0
 * Version:     1.0
 */

#include <MIDI.h>
#include <MIDIUSB.h>
#include <FastLED.h>
#include <DmxSimple.h>

#define LED 13

#define MIDI_THRU       true
#if MIDI_THRU
#define MIDI_CHANNEL    MIDI_CHANNEL_OMNI
#else
#define MIDI_CHANNEL    9
#endif

#define ARGB_PIN        2
#define ARGB_LEDS       12
#define ARGB_BRIGHTNESS 64
#define ARGB_TYPE       WS2812B
#define ARGB_ORDER      GRB
CRGB leds[ARGB_LEDS];

#define RGB_R           9
#define RGB_G           10
#define RGB_B           11

#define DMX_PIN         5
#define DMX_CHANNELS    50
#define DMX_R           1
#define DMX_G           2
#define DMX_B           3

#define COLOR_UPDATE    10
#define PALETTE_UPDATE  40
#define PALETTE_CHANGES 24
enum ColorMode {
    Unknown,
    SolidRgb,
    SolidHsv,
    Palette
};
#define COLOR_MODES     5
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
ColorMode colorMode = SolidRgb;
CRGB color_rgb = CRGB::Black;
CHSV color_hsv(0, 0, 0);
CRGBPalette16 current_palette(CRGB::Black);
TBlendType blending = LINEARBLEND;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {

    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    delay(3000); // Power On Delay

    MIDI.begin(MIDI_CHANNEL);
    MIDI.setHandleControlChange(controlChange);
    MIDI.setHandleNoteOn(noteOn);
    MIDI.setHandleNoteOff(noteOff);

    FastLED.addLeds<ARGB_TYPE, ARGB_PIN, ARGB_ORDER>(leds, ARGB_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(ARGB_BRIGHTNESS);

    pinMode(RGB_R, OUTPUT);
    analogWrite(RGB_R, 0);
    pinMode(RGB_G, OUTPUT);
    analogWrite(RGB_G, 0);
    pinMode(RGB_B, OUTPUT);
    analogWrite(RGB_B, 0);

    DmxSimple.usePin(4);
    DmxSimple.maxChannel(DMX_CHANNELS);

    digitalWrite(LED, LOW); // Indicate that initialization is complete

}

CRGBPalette16 target_palette(CRGB::Black);
uint8_t selected_palette = 0;
unsigned long palette_millis;
uint8_t palette_index = 0;
float palette_pos = 0;
float palette_speed = 1;

void loop() {
    EVERY_N_MILLISECONDS(COLOR_UPDATE) {
        palette_pos += palette_speed;
        if (palette_pos >= 256.0f) palette_pos -= 256.0f;
        palette_index = (uint8_t)palette_pos;

        switch (colorMode) {
            default:
                FillLEDsFromPaletteColors(palette_index);
                break;
        }

        FastLED.show();
    }

    EVERY_N_MILLISECONDS(PALETTE_UPDATE) {
        nblendPaletteTowardPalette(current_palette, target_palette, PALETTE_CHANGES);
    }

    MIDI.read();
    MidiUSB.read();
}

void FillLEDsFromPaletteColors(uint8_t index) {
    uint8_t brightness = 255;
    CRGB c = ColorFromPalette(current_palette, index, brightness, blending);

    analogWrite(RGB_R, c.r);
    analogWrite(RGB_G, c.g);
    analogWrite(RGB_B, c.b);

    DmxSimple.write(DMX_R, c.r);
    DmxSimple.write(DMX_G, c.g);
    DmxSimple.write(DMX_B, c.b);

    leds[0] = c;
    for (int i = 1; i < ARGB_LEDS; i++) {
        index += 3;
        leds[i] = ColorFromPalette(current_palette, index, brightness, blending);
    }
}

void update_palette() {
    switch (colorMode) {
        case SolidRgb:
            target_palette = CRGBPalette16(color_rgb);
            break;
        case SolidHsv:
            target_palette = CRGBPalette16(color_hsv);
            break;
        case Palette:
            target_palette = *palettes[selected_palette];
            break;
    }
}

// Midi Events

void controlChange(byte channel, byte control, byte value) {
    #if MIDI_THRU
    midiEventPacket_t event = {0x08, 0x80 | channel, control, value};
    MidiUSB.sendMIDI(event);
    #endif
}

void noteOn(byte channel, byte note, byte velocity) {
    #if MIDI_THRU
    midiEventPacket_t event = {0x09, 0x90 | channel, note, velocity};
    MidiUSB.sendMIDI(event);
    #endif

    if (MIDI_CHANNEL > 0 && channel != MIDI_CHANNEL) return;
    digitalWrite(LED, HIGH);

    if (velocity == 0) {
        noteOff(channel, note, velocity);
        return;
    }

    switch (note) {

        // Type
        case 0:
            if (velocity < COLOR_MODES) colorMode = (ColorMode)velocity;
            break;

        // Red Intensity
        case 1:
            color_rgb.r = velocity*2;
            break;

        // Green Intensity
        case 2:
            color_rgb.g = velocity*2;
            break;

        // Blue Intensity
        case 3:
            color_rgb.b = velocity*2;
            break;

        // Hue
        case 4:
            color_hsv.h = velocity*2;
            break;

        // Saturation
        case 5:
            color_hsv.s = velocity*2;
            break;

        // Value
        case 6:
            color_hsv.v = velocity*2;
            break;

        // Palette Index
        case 7:
            if (velocity < palette_count) selected_palette = velocity;
            break;

        // Palette Speed
        case 8:
            palette_speed = (float)velocity / 32.0f;

    }

    update_palette();
}

void noteOff(byte channel, byte note, byte velocity) {
    #if MIDI_THRU
    midiEventPacket_t event = {0x0b, 0xb0 | channel, note, velocity};
    MidiUSB.sendMIDI(event);
    #endif

    if (MIDI_CHANNEL > 0 && channel != MIDI_CHANNEL) return;
    digitalWrite(LED, LOW);

    switch (note) {

        // Red Intensity
        case 60: // C3
            analogWrite(RGB_R, 0);
            DmxSimple.write(DMX_R, 0);
            break;

    }
}
