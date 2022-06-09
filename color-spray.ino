/**
 * Title:       Color Spray
 * Created by:  Cooper Dalrymple
 * Date:        2022-05-06
 * License:     GNU GPL v3.0
 * Version:     1.0
 */

#define USB_MIDI        true
#define USB_TIMEOUT     2000

#if USB_MIDI
#include <USB-MIDI.h>
#else
#include <MIDI.h>
#endif
USING_NAMESPACE_MIDI;

struct CustomMidiSettings : public MIDI_NAMESPACE::DefaultSettings {
    static const unsigned SysExMaxSize = 32;
};
typedef Message<CustomMidiSettings::SysExMaxSize> MidiMessage;
typedef MIDI_NAMESPACE::MidiType MidiType;

#if USB_MIDI
#define UsbMidiMessage MIDI_NAMESPACE::MidiInterface<USBMIDI_NAMESPACE::usbMidiTransport, CustomMidiSettings>::MidiMessage
USBMIDI_CREATE_CUSTOM_INSTANCE(0, MIDICoreUSB, CustomMidiSettings);
#endif

#define SerialMidiMessage MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial>, CustomMidiSettings>::MidiMessage
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, MIDICoreSerial, CustomMidiSettings);

#include <FastLED.h>
#include <DmxSimple.h> // Included after FastLED.h to prevent loading of default DMX controller
#include "dmx.h"

#include "sysex.h"
#include "color.h"
#include "palettes.h"
#include "preset.h"
#include "settings.h"

#define LED             13
#define LED_DURATION    10

#define ARGB_PIN        2
#define ARGB_ORDER      GRB
#define ARGB_TYPE       WS2812B
#define ARGB_MAX        120
static CRGB argbLights[ARGB_MAX];
static ARGB_TYPE<ARGB_PIN, ARGB_ORDER> argbController;
// static ARGB_TYPE<ARGB_PIN> argbController; // For NEOPIXEL
uint8_t modArgb = 1;

#define DMX_PIN         7
#define DMX_ORDER       RGB
#define DMX_MAX         32
static CRGB dmxLights[DMX_MAX];
static DMXSIMPLE<DMX_PIN, DMX_ORDER> dmxController;
uint8_t modDmx = 1;

#define RGB_R           9
#define RGB_G           10
#define RGB_B           11
#define RGB_W           5

#define COLOR_UPDATE    10
#define PALETTE_UPDATE  1
#define PALETTE_CHANGES 48
#define PALETTE_ITERS   1
#define PALETTE_MAX     16

PresetData current_preset = default_preset;
CRGBPalette16 current_palette(CRGB::Black);
TBlendType blending = LINEARBLEND;

Settings settings;

void configure();
void loadPreset(uint8_t i);

#if USB_MIDI
void messageUartToUsb(MidiMessage message);
void messageUsbToUart(MidiMessage message);
#endif
void controlChange(byte channel, byte control, byte value);
void programChange(byte channel, byte program);

void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);

    pinMode(0, INPUT);
    pinMode(1, OUTPUT);

    pinMode(RGB_R, OUTPUT);
    analogWrite(RGB_R, 0);
    pinMode(RGB_G, OUTPUT);
    analogWrite(RGB_G, 0);
    pinMode(RGB_B, OUTPUT);
    analogWrite(RGB_B, 0);
    pinMode(RGB_W, OUTPUT);
    analogWrite(RGB_W, 0);

    #if USB_MIDI
    #else
    Serial.begin(9600);
    #endif

    #if USB_MIDI
    MIDICoreSerial.setHandleMessage(messageUartToUsb);
    #endif
    MIDICoreSerial.setHandleControlChange(controlChange);
    MIDICoreSerial.setHandleProgramChange(programChange);
    MIDICoreSerial.setHandleNoteOn(noteOn);
    MIDICoreSerial.setHandleSystemExclusive(systemExclusive);

    #if USB_MIDI
    MIDICoreUSB.turnThruOff();
    MIDICoreUSB.setHandleMessage(messageUsbToUart);
    MIDICoreUSB.setHandleControlChange(controlChange);
    MIDICoreUSB.setHandleProgramChange(programChange);
    MIDICoreUSB.setHandleNoteOn(noteOn);
    MIDICoreUSB.setHandleSystemExclusive(systemExclusive);
    #endif

    MIDICoreSerial.begin(MIDI_CHANNEL_OMNI);
    #if USB_MIDI
    MIDICoreUSB.begin(MIDI_CHANNEL_OMNI);
    #endif

    configure();

    digitalWrite(LED, LOW); // Indicate that initialization is complete
}

bool firstConfigure = true;
void configure() {
    if (!firstConfigure) FastLED.clearData();

    #if USB_MIDI
    #else
    unsigned long usb_start = millis();
    while (!Serial && millis() - usb_start < USB_TIMEOUT) { }
    if (Serial) {
        Serial.println(F("Color Spray - Version 1.0 - 2022 Cooper Dalrymple"));

        Serial.print(F("Midi Thru: "));
        if (settings.data.midiThru) {
            Serial.print(F("On"));
        } else {
            Serial.print(F("Off"));
        }
        Serial.println();

        Serial.print(F("Midi Channel: "));
        if (settings.data.midiChannel > 0) {
            Serial.print(settings.data.midiChannel);
        } else {
            Serial.print(F("Omni"));
        }
        Serial.println();
    }
    #endif

    if (settings.data.midiThru) {
        MIDICoreSerial.turnThruOn();
    } else {
        MIDICoreSerial.turnThruOff();
    }

    if (firstConfigure) {
        argbController.init();
        argbController.setCorrection(TypicalLEDStrip);
    }
    if (settings.data.argbCount > 0) {
        modArgb = max(floor((float)min(settings.data.dmxCount, DMX_MAX) / (float)min(settings.data.argbCount, ARGB_MAX)), 1);
        argbController.setLeds(argbLights, min(settings.data.argbCount, ARGB_MAX));
    } else {
        argbController.setLeds(NULL, 0);
    }

    if (firstConfigure) {
        dmxController.init();
        dmxController.assign(&settings);
    }
    if (settings.data.dmxCount > 0) {
        modDmx = max(floor((float)min(settings.data.argbCount, ARGB_MAX) / (float)min(settings.data.dmxCount, DMX_MAX)), 1);
        dmxController.setLeds(dmxLights, min(settings.data.dmxCount, DMX_MAX));
    } else {
        dmxController.setLeds(NULL, 0);
    }
    dmxController.update();

    FastLED.setBrightness(settings.data.brightness);

    if (settings.data.preset > 0) {
        loadPreset(settings.data.preset - 1);
    }

    firstConfigure = false;
}

CRGBPalette16 target_palette(CRGB::Black);
unsigned long palette_millis;
uint8_t palette_index = 0;
float palette_pos = 0;

unsigned long led_millis = -1;

#if USB_MIDI
void messageUartToUsb(MidiMessage message) {
    MIDICoreUSB.send(message);
}
void messageUsbToUart(MidiMessage message) {
    MIDICoreSerial.send(message);
}
#endif

void loop() {
    EVERY_N_MILLISECONDS(COLOR_UPDATE) {
        palette_pos += current_preset.palette_speed / 32.0f;
        if (palette_pos >= 256.0f) palette_pos -= 256.0f;
        palette_index = (uint8_t)palette_pos;

        switch (current_preset.mode) {
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

    MIDICoreSerial.read();
    #if USB_MIDI
    MIDICoreUSB.read();
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

    uint8_t iArgb = 1;
    uint8_t iDmx = 1;
    for (int i = 1; i < max(min(settings.data.argbCount, ARGB_MAX), min(settings.data.dmxCount, DMX_MAX)); i++) {
        index += 3;
        c = ColorFromPalette(current_palette, index, brightness, blending);
        if (i % modArgb == 0 && iArgb < min(settings.data.argbCount, ARGB_MAX)) argbLights[iArgb++] = c;
        if (i % modDmx == 0 && iDmx < min(settings.data.dmxCount, DMX_MAX)) dmxLights[iDmx++] = c;
    }
}

void updatePalette() {
    switch (current_preset.mode) {
        case ColorOff:
            target_palette = CRGBPalette16(CRGB::Black);
        case SolidRgb:
            target_palette = CRGBPalette16(current_preset.color_rgb);
            break;
        case SolidHsv:
            target_palette = CRGBPalette16(current_preset.color_hsv);
            break;
        case Palette:
            target_palette = *palettes[current_preset.palette];
            break;
        case ChaseRgb:
        case ChaseHsv:
            CRGB *entries = new CRGB[PALETTE_MAX];
            for (uint8_t i = 0; i < PALETTE_MAX; i++) {
                entries[i] = CRGB::Black;
            }
            uint8_t count = constrain(current_preset.chase_count, 1, 8);
            uint8_t step = PALETTE_MAX / count;
            for (uint8_t i = 0; i < PALETTE_MAX; i += step) {
                if (current_preset.mode == ChaseRgb) {
                    entries[i] = current_preset.color_rgb;
                } else {
                    entries[i] = current_preset.color_hsv;
                }
            }
            target_palette = CRGBPalette16(entries);
            delete entries;
            break;
    }
}

void loadPreset(uint8_t i) {
    settings.presetcpy(&current_preset, i);
    updatePalette();
}

// Midi Events

bool updated = false;
void controlChange(byte channel, byte control, byte value) {
    if (settings.data.midiChannel > 0 && channel != settings.data.midiChannel - 1) return;

    updated = false;
    switch (control) {

        // Palette Controls (Effect Controller 1-2)
        case 12:
            current_preset.palette = map(value, 0, 127, 0, PALETTE_COUNT);
            if (current_preset.palette >= PALETTE_COUNT) current_preset.palette = PALETTE_COUNT - 1;
            updated = true;
            break;
        case 13:
            current_preset.palette_speed = value;
            updated = true;
            break;

        // RGB Controls (General Purpose Controller 1-4)
        case 16:
            current_preset.color_rgb.r = value*2;
            updated = true;
            break;
        case 17:
            current_preset.color_rgb.g = value*2;
            updated = true;
            break;
        case 18:
            current_preset.color_rgb.b = value*2;
            updated = true;
            break;

        // HSV Controls (General Purpose Controller 5-8)
        case 80:
            current_preset.color_hsv.h = value*2;
            updated = true;
            break;
        case 81:
            current_preset.color_hsv.s = value*2;
            updated = true;
            break;
        case 82:
            current_preset.color_hsv.v = value*2;
            updated = true;
            break;

        // Chase Controls
        case 83:
            current_preset.chase_count = map(value, 0, 127, 0, 7);
            if (current_preset.chase_count > 7) current_preset.chase_count = 7;
            current_preset.chase_count++;
            updated = true;
            break;

    }

    if (updated) {
        triggerLed();
        updatePalette();
    }
}

void programChange(byte channel, byte program) {
    if (settings.data.midiChannel > 0 && channel != settings.data.midiChannel - 1) return;

    if (program == 0) {
        current_preset.mode = ColorOff;
    } else if (program < 1 + PRESET_COUNT) {
        loadPreset(program - 1);
    } else if (program < 1 + PRESET_COUNT + COLOR_MODES - 1) {
        current_preset.mode = (ColorMode)(program - 1 - PRESET_COUNT + 1);
    }

    if (program < PRESET_COUNT + COLOR_MODES) {
        triggerLed();
        updatePalette();
    }
}

void noteOn(byte channel, byte note, byte velocity) {
    if (settings.data.midiChannel > 0 && channel != settings.data.midiChannel - 1) return;
    if (velocity == 0) return;

    for (uint8_t i = 0; i < PRESET_COUNT; i++) {
        if (settings.presets[i].note == note) {
            loadPreset(i);
            triggerLed();
            break;
        }
    }
}

void systemExclusive(byte* data, unsigned size) {
    bool valid = size >= 5 && data[1] == MANUFACTURER_ID && data[2] == DEVICE_ID;

    #if USB_MIDI
    #else
    if (Serial) {

        Serial.print(F("Sysex Message Received ("));
        Serial.print(size);
        Serial.println(F(" bytes in total):"));
        if (valid) {
            Serial.print(F("Message is Valid"));
        } else {
            Serial.print(F("Invalid Message"));
        }
        Serial.println();
        Serial.println(F("Message Contents:"));
        for (uint8_t i = 0; i < size; i++) {
            Serial.print("0x");
            Serial.print(data[i], HEX);
            Serial.print(" ");
            if ((i > 0 && i % 8 == 0) || i == size - 1) {
                Serial.println();
            }
        }
        Serial.println();
    }
    #endif

    if (!valid) return;
    triggerLed();

    unsigned response_size = 3;
    switch (data[0x03]) {
        case COMMAND_READ_VERSION:
            response_size = 4;
            break;
        case COMMAND_READ_SETTINGS:
            response_size = 4 + sizeof(SettingsData);
            break;
        case COMMAND_READ_PRESET:
            if (size >= 5 && data[0x04] < PRESET_COUNT) {
                response_size = 4 + sizeof(PresetData);
            }
            break;
    }
    byte* response = new byte[response_size];
    response[0x00] = MANUFACTURER_ID;
    response[0x01] = DEVICE_ID;
    response[0x02] = RESPONSE_ERROR;

    switch (data[0x03]) {
        case COMMAND_ALIVE:
            response[0x02] = RESPONSE_SUCCESS;
            break;
        case COMMAND_RESET:
            configure();
            response[0x02] = RESPONSE_SUCCESS;
            break;

        case COMMAND_READ_VERSION:
            response[0x02] = RESPONSE_SUCCESS;
            response[0x03] = SETTINGS_VERSION;
            break;
        case COMMAND_READ_SETTINGS:
            response[0x02] = RESPONSE_SUCCESS;
            response[0x03] = SETTINGS_VERSION;
            settings.datacpy(response+4, true);
            break;
        case COMMAND_READ_PRESET:
            if (size >= 5 && data[0x04] < PRESET_COUNT) {
                response[0x02] = RESPONSE_SUCCESS;
                response[0x03] = SETTINGS_VERSION;
                settings.presetcpy(response+4, data[0x04], true);
            }
            break;

        case COMMAND_WRITE_SETTINGS:
            if (size == 5 + sizeof(SettingsData) + 1 && data[0x04] == SETTINGS_VERSION) {
                settings.datawrite(data+5, true);
                response[0x02] = RESPONSE_SUCCESS;
            }
            break;
        case COMMAND_WRITE_PRESET:
            if (size == 6 + sizeof(PresetData) + 1 && data[0x04] == SETTINGS_VERSION && data[0x05] < PRESET_COUNT) {
                settings.presetwrite(data+6, data[0x05], true);
                response[0x02] = RESPONSE_SUCCESS;
            }
            break;
    }

    MIDICoreSerial.sendSysEx(response_size, response);
    #if USB_MIDI
    MIDICoreUSB.sendSysEx(response_size, response);
    #endif
    delete response;
}
