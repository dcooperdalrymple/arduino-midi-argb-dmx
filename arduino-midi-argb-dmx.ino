/**
 * Title:       Arduino MIDI ARGB DMX
 * Created by:  Cooper Dalrymple
 * Date:        2022-05-06
 * License:     GNU GPL v3.0
 * Version:     1.0
 */

#define USB_MIDI        true
#define USB_TIMEOUT     2000

#include <MIDI.h>
#if USB_MIDI
#include <MIDIUSB.h>
#endif
#include <DmxSimple.h>
#include <FastLED.h>

#include "settings.h"

#define LED             13
#define LED_DURATION    10

#define ARGB_PIN        2
#define ARGB_ORDER      GRB
CRGB *argbLights;

#define DMX_PIN         7
#define DMX_ORDER       GBR
CRGB *dmxLights;

#define RGB_R           9
#define RGB_G           10
#define RGB_B           11
#define RGB_W           5

#define COLOR_UPDATE    10
#define PALETTE_UPDATE  1
#define PALETTE_CHANGES 48
#define PALETTE_ITERS   1
enum ColorMode : uint8_t {
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
#define MidiMessage MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>>::MidiMessage
#define MidiType MIDI_NAMESPACE::MidiType

Settings settings;

#if USB_MIDI
void messageUartToUsb(MidiMessage message);
#endif
void controlChange(byte channel, byte control, byte value);
void programChange(byte channel, byte program);

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
        if (settings.getMidiThru()) {
            Serial.print("On");
        } else {
            Serial.print("Off");
        }
        Serial.println();

        Serial.print("Midi Channel: ");
        if (settings.getMidiChannel() > 0) {
            Serial.print(settings.getMidiChannel());
        } else {
            Serial.print("Omni");
        }
        Serial.println();
    }
    #endif

    pinMode(0, INPUT);
    pinMode(1, OUTPUT);
    if (settings.getMidiThru()) {
        MIDI.turnThruOn();
    } else {
        MIDI.turnThruOff();
    }
    #if USB_MIDI
    MIDI.setHandleMessage(messageUartToUsb);
    #endif
    MIDI.setHandleControlChange(controlChange);
    MIDI.setHandleProgramChange(programChange);
    MIDI.begin(MIDI_CHANNEL_OMNI);

    argbLights = new CRGB[settings.getArgbCount()];
    switch (settings.getArgbType()) {
        case FLTYPE_NEOPIXEL:
            FastLED.addLeds<NEOPIXEL, ARGB_PIN>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_SM16703:
            //FastLED.addLeds<SM16703, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_TM1829:
            //FastLED.addLeds<TM1829, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_TM1812:
            //FastLED.addLeds<TM1812, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_TM1809:
            FastLED.addLeds<TM1809, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_TM1804:
            FastLED.addLeds<TM1804, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_TM1803:
            FastLED.addLeds<TM1803, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_UCS1903:
            FastLED.addLeds<UCS1903, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_UCS1903B:
            //FastLED.addLeds<UCS1903B, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_UCS1904:
            //FastLED.addLeds<UCS1904, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_UCS2903:
            //FastLED.addLeds<UCS2903, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_WS2812:
            FastLED.addLeds<WS2812, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_WS2852:
            //FastLED.addLeds<WS2852, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_WS2812B:
            FastLED.addLeds<WS2812B, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_GS1903:
            //FastLED.addLeds<GS1903, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_SK6812:
            //FastLED.addLeds<SK6812, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_SK6822:
            //FastLED.addLeds<SK6822, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_APA106:
            //FastLED.addLeds<APA106, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_PL9823:
            //FastLED.addLeds<PL9823, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_WS2811:
            FastLED.addLeds<WS2811, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_WS2813:
            FastLED.addLeds<WS2813, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_APA104:
            //FastLED.addLeds<APA104, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_WS2811_400:
            //FastLED.addLeds<WS2811_400, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_GE8822:
            //FastLED.addLeds<GE8822, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_GW6205:
            FastLED.addLeds<GW6205, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_GW6205_400:
            //FastLED.addLeds<GW6205_400, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_LPD1886:
            //FastLED.addLeds<LPD1886, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_LPD1886_8BIT:
            //FastLED.addLeds<LPD1886_8BIT, ARGB_PIN, ARGB_ORDER>(argbLights, settings.getArgbCount()).setCorrection(TypicalLEDStrip);
            break;
        case FLTYPE_NONE:
        default:
            break;
    }

    dmxLights = new CRGB[settings.getDmxCount()];
    FastLED.addLeds<DMXSIMPLE, DMX_PIN, DMX_ORDER>(dmxLights, settings.getDmxCount());

    FastLED.setBrightness(settings.getBrightness());

    pinMode(RGB_R, OUTPUT);
    analogWrite(RGB_R, 0);
    pinMode(RGB_G, OUTPUT);
    analogWrite(RGB_G, 0);
    pinMode(RGB_B, OUTPUT);
    analogWrite(RGB_B, 0);
    pinMode(RGB_W, OUTPUT);
    analogWrite(RGB_W, 0);

    digitalWrite(LED, LOW); // Indicate that initialization is complete

}

CRGBPalette16 target_palette(CRGB::Black);
uint8_t selected_palette = 0;
unsigned long palette_millis;
uint8_t palette_index = 0;
float palette_pos = 0;
float palette_speed = 1;

unsigned long led_millis = -1;

#if USB_MIDI
void messageUartToUsb(MidiMessage message) {
    midiEventPacket_t event;
    event.header = 0xff;
    switch (message.type) {
        case MidiType::ProgramChange: // Program Change
        case MidiType::AfterTouchChannel: // Channel Pressure
            event.header = (message.type >> 4) & 0x0f;
            event.byte1 = (message.type & 0xf0) | (message.channel & 0x0f);
            event.byte2 = message.data1 & 0x7f;
            break;
        case MidiType::NoteOff:
        case MidiType::NoteOn:
        case MidiType::AfterTouchPoly:
        case MidiType::ControlChange:
        case MidiType::PitchBend:
            event.header = (message.type >> 4) & 0x0f;
            event.byte1 = (message.type & 0xf0) | (message.channel & 0x0f);
            event.byte2 = message.data1 & 0x7f;
            event.byte3 = message.data2 & 0x7f;
            break;
        case MidiType::TuneRequest:
        case MidiType::Clock:
        case MidiType::Start:
        case MidiType::Continue:
        case MidiType::Stop:
            event.header = (message.type >> 4) & 0x0f;
            event.byte1 = message.type;
            break;
    }
    if (event.header != 0xff) {
        MidiUSB.sendMIDI(event);
        MidiUSB.flush();
    }
}
#endif

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
    midiEventPacket_t event;
    do {
        event = MidiUSB.read();
        switch (event.header & 0x0f) {
            case 0x0b:
                controlChange(event.byte1 & 0x0f, event.byte2 & 0x7f, event.byte3 & 0x7f);
                break;
            case 0x0c:
                programChange(event.byte1 & 0x0f, event.byte2 & 0x7f);
                break;
        }
        if (settings.getMidiThru()) {
            switch (event.header & 0x0f) {
                case 0x0c: // Program Change
                case 0x0d: // Channel Pressure
                    MIDI.send((MidiType)(event.header << 4), event.byte2, 0, event.byte1 & 0x0f);
                    break;
                case 0x08: // Note-off
                case 0x09: // Note-on
                case 0x0a: // Poly-KeyPress
                case 0x0b: // Control Change
                case 0x0e: // PitchBend Change
                    MIDI.send((MidiType)(event.header << 4), event.byte2, event.byte3, event.byte1 & 0x0f);
                    break;
                case 0x0f: // Single Byte, TuneRequest, Clock, Start, Continue, Stop, etc.
                    MIDI.sendCommon((MidiType)event.byte1, 0);
                    break;
            }
        }
    } while (event.header != 0);
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

    argbLights[0] = c;
    dmxLights[0] = c;
    for (int i = 1; i < max(settings.getArgbCount(), settings.getDmxCount()); i++) {
        index += 3;
        c = ColorFromPalette(current_palette, index, brightness, blending);
        if (i < settings.getArgbCount()) argbLights[i] = c;
        if (i < settings.getDmxCount()) dmxLights[i] = c;
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
    if (settings.getMidiChannel() > 0 && channel != settings.getMidiChannel() - 1) return;

    updated = false;
    switch (control) {

        // Palette Controls (Effect Controller 1-2)
        case 12:
            selected_palette = map(value, 0, 127, 0, palette_count);
            if (selected_palette >= palette_count) selected_palette = palette_count - 1;
            updated = true;
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
    if (settings.getMidiChannel() > 0 && channel != settings.getMidiChannel() - 1) return;

    if (program < COLOR_MODES) {
        colorMode = (ColorMode)program;
        triggerLed();
        updatePalette();
    }
}
