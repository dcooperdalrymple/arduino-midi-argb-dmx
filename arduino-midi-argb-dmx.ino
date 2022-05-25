/**
 * Title:       Arduino MIDI ARGB DMX
 * Created by:  Cooper Dalrymple
 * Date:        2022-05-06
 * License:     GNU GPL v3.0
 * Version:     1.0
 */

#define USB_MIDI        false
#define USB_TIMEOUT     2000

#include <MIDI.h>
#if USB_MIDI
#include <MIDIUSB.h>
#endif
#include <FastLED.h>
#include <DmxSimple.h>

#define LED             13
#define LED_DURATION    10

#define MIDI_THRU       true
#define MIDI_CHANNEL    MIDI_CHANNEL_OMNI

#define ARGB_PIN        2
#define ARGB_LEDS       12
#define ARGB_BRIGHTNESS 64
#define ARGB_TYPE       WS2812B
#define ARGB_ORDER      GRB
CRGB leds[ARGB_LEDS];

#define RGB_R           9
#define RGB_G           10
#define RGB_B           11
#define RGB_W           5

#define DMX_PIN         7
#define DMX_CHANNELS    50
#define DMX_R           1
#define DMX_G           2
#define DMX_B           3

#define COLOR_UPDATE    10
#define PALETTE_UPDATE  1
#define PALETTE_CHANGES 48
#define PALETTE_ITERS   1
enum ColorMode {
    ColorOff,
    SolidRgb,
    SolidHsv,
    Palette
};
#define COLOR_MODES     4
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
ColorMode colorMode = ColorOff;
CRGB color_rgb = CRGB::Black;
CHSV color_hsv(0, 0, 0);
CRGBPalette16 current_palette(CRGB::Black);
TBlendType blending = LINEARBLEND;

MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {

    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    #if USB_MIDI
    #else
    Serial.begin(9600);
    unsigned long usb_start = millis();
    while (!Serial && millis() - usb_start < USB_TIMEOUT) { }
    if (Serial) {
        Serial.println("Arduino MIDI ARGB DMX - Version 1.0 - 2022 Cooper Dalrymple");

        Serial.print("Midi Thru: ");
        #if MIDI_THRU
        Serial.print("On");
        #else
        Serial.print("Off");
        #endif
        Serial.println();

        Serial.print("Midi Channel: ");
        if (MIDI_CHANNEL > 0) {
            Serial.print(MIDI_CHANNEL);
        } else {
            Serial.print("Omni");
        }
        Serial.println();
    }
    #endif

    MIDI.begin(MIDI_CHANNEL);
    MIDI.setHandleControlChange(controlChange);
    MIDI.setHandleNoteOn(noteOn);
    MIDI.setHandleNoteOff(noteOff);
    MIDI.setHandleProgramChange(programChange);

    FastLED.addLeds<ARGB_TYPE, ARGB_PIN, ARGB_ORDER>(leds, ARGB_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(ARGB_BRIGHTNESS);

    pinMode(RGB_R, OUTPUT);
    analogWrite(RGB_R, 0);
    pinMode(RGB_G, OUTPUT);
    analogWrite(RGB_G, 0);
    pinMode(RGB_B, OUTPUT);
    analogWrite(RGB_B, 0);
    pinMode(RGB_W, OUTPUT);
    analogWrite(RGB_W, 0);

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

unsigned long led_millis = -1;

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
        #if PALETTE_ITERS > 1
        for (uint8_t i = 0; i < PALETTE_ITERS; i++) {
            nblendPaletteTowardPalette(current_palette, target_palette, PALETTE_CHANGES);
        }
        #else
        nblendPaletteTowardPalette(current_palette, target_palette, PALETTE_CHANGES);
        #endif
    }

    if (led_millis >= 0 && millis() - led_millis > LED_DURATION) {
        digitalWrite(LED, LOW);
        led_millis = -1;
    }

    MIDI.read();
    #if USB_MIDI
    MidiUSB.read();
    #endif
}

void triggerLed() {
    digitalWrite(LED, HIGH);
    led_millis = millis();
}

// LED Functions

uint8_t getW(CRGB color) {
    return min(color.r, min(color.g, color.b));
}

void FillLEDsFromPaletteColors(uint8_t index) {
    uint8_t brightness = 255;
    CRGB c = ColorFromPalette(current_palette, index, brightness, blending);

    analogWrite(RGB_R, c.r);
    analogWrite(RGB_G, c.g);
    analogWrite(RGB_B, c.b);
    analogWrite(RGB_W, getW(c));

    DmxSimple.write(DMX_R, c.r);
    DmxSimple.write(DMX_G, c.g);
    DmxSimple.write(DMX_B, c.b);

    leds[0] = c;
    for (int i = 1; i < ARGB_LEDS; i++) {
        index += 3;
        leds[i] = ColorFromPalette(current_palette, index, brightness, blending);
    }
}

void updatePalette() {
    switch (colorMode) {
        case ColorOff:
            target_palette = CRGBPalette16(CRGB::Black);
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

bool updated = false;
void controlChange(byte channel, byte control, byte value) {
    #if MIDI_THRU
    MIDI.sendControlChange(control, value, channel);
    #if USB_MIDI
    midiEventPacket_t event = {0xb0 | channel, control, value};
    MidiUSB.sendMIDI(event);
    #endif
    #endif

    if (MIDI_CHANNEL > 0 && channel != MIDI_CHANNEL - 1) return;

    updated = false;
    switch (control) {

        // Palette Controls (Effect Controller 1-2)
        case 12:
            if (value < palette_count) {
                selected_palette = value;
                updated = true;
            }
            break;
        case 13:
            palette_speed = (float)value / 32.0f;
            updated = true;
            break;

        // RGB Controls (General Purpose Controller 1-4)
        case 16:
            color_rgb.r = value*2;
            updated = true;
            break;
        case 17:
            color_rgb.g = value*2;
            updated = true;
            break;
        case 18:
            color_rgb.b = value*2;
            updated = true;
            break;

        // HSV Controls (General Purpose Controller 5-8)
        case 80:
            color_hsv.h = value*2;
            updated = true;
            break;
        case 81:
            color_hsv.s = value*2;
            updated = true;
            break;
        case 82:
            color_hsv.v = value*2;
            updated = true;
            break;

    }

    if (updated) {
        triggerLed();
        updatePalette();
    }
}

void programChange(byte channel, byte program) {
    #if MIDI_THRU
    MIDI.sendProgramChange(program, channel);
    #if USB_MIDI
    midiEventPacket_t event = {0xc0 | channel, program};
    MidiUSB.sendMIDI(event);
    #endif
    #endif

    if (MIDI_CHANNEL > 0 && channel != MIDI_CHANNEL - 1) return;

    if (program < COLOR_MODES) {
        colorMode = (ColorMode)program;
        triggerLed();
        updatePalette();
    }
}

void noteOn(byte channel, byte note, byte velocity) {
    #if MIDI_THRU
    MIDI.sendNoteOn(note, velocity, channel);
    #if USB_MIDI
    midiEventPacket_t event = {0x90 | channel, note, velocity};
    MidiUSB.sendMIDI(event);
    #endif
    #endif
}

void noteOff(byte channel, byte note, byte velocity) {
    #if MIDI_THRU
    MIDI.sendNoteOff(note, velocity, channel);
    #if USB_MIDI
    midiEventPacket_t event = {0x80 | channel, note, velocity};
    MidiUSB.sendMIDI(event);
    #endif
    #endif
}
