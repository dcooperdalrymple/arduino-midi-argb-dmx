#ifndef SETTINGS_H
#define SETTINGS_H

#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <MIDI.h>
#include <FastLED.h>

#include "sysex.h"
#include "preset.h"

#define SETTINGS_VERSION    3
#define SETTINGS_OFFSET     0x03
#define PRESET_OFFSET       0x10
#define PRESET_SIZE         0x10
#define PRESET_COUNT        15

enum FastLedType : uint8_t {
    FLTYPE_NONE,
    FLTYPE_NEOPIXEL,
    FLTYPE_SM16703,
    FLTYPE_TM1829,
    FLTYPE_TM1812,
    FLTYPE_TM1809,
    FLTYPE_TM1804,
    FLTYPE_TM1803,
    FLTYPE_UCS1903,
    FLTYPE_UCS1903B,
    FLTYPE_UCS1904,
    FLTYPE_UCS2903,
    FLTYPE_WS2812,
    FLTYPE_WS2852,
    FLTYPE_WS2812B,
    FLTYPE_GS1903,
    FLTYPE_SK6812,
    FLTYPE_SK6822,
    FLTYPE_APA106,
    FLTYPE_PL9823,
    FLTYPE_WS2811,
    FLTYPE_WS2813,
    FLTYPE_APA104,
    FLTYPE_WS2811_400,
    FLTYPE_GE8822,
    FLTYPE_GW6205,
    FLTYPE_GW6205_400,
    FLTYPE_LPD1886,
    FLTYPE_LPD1886_8BIT
};

typedef struct _SettingsData {
    uint8_t midiChannel;
    bool midiThru;
    uint8_t brightness;
    uint8_t argbCount;
    FastLedType argbType;
    uint8_t dmxCount;
    uint8_t dmxChannelSize;
    uint8_t dmxChannelOffset;
    uint8_t dmxBrightness;
    uint8_t dmxBrightnessChannel;
    uint8_t preset;
} SettingsData;

const PROGMEM SettingsData default_settings = {
    .midiChannel = MIDI_CHANNEL_OMNI,
    .midiThru = true,
    .brightness = 64,
    .argbCount = 12,
    .argbType = FLTYPE_WS2812B,
    .dmxCount = 1,
    .dmxChannelSize = 3,
    .dmxChannelOffset = 1,
    .dmxBrightness = 255,
    .dmxBrightnessChannel = 0,
    .preset = 0
};

class Settings {

private:
    SettingsData _data;
    PresetData* _presets;

    uint8_t read_version();
    bool verify();
    bool read_data();
    void write_settings();
    void write_preset(uint8_t i);
    void write_data();
    void reset();

public:
    Settings();

    uint8_t getMidiChannel();
    bool getMidiThru();

    uint8_t getBrightness();

    uint8_t getArgbCount();
    FastLedType getArgbType();

    uint8_t getDmxCount();
    uint8_t getDmxChannelSize();
    uint8_t getDmxChannelOffset();
    uint8_t getDmxBrightness();
    uint8_t getDmxBrightnessChannel();

    uint8_t getPreset();
    PresetData* getPresetData(uint8_t i);

    bool handleSysex(byte* data, unsigned size);

    void datacpy(void * destination, bool adjust = false);
    void presetcpy(void * destination, uint8_t i, bool adjust = false);

    void datawrite(void * source, bool adjust = false);
    void presetwrite(void * source, uint8_t i, bool adjust = false);

};

#endif
