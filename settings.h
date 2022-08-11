#ifndef SETTINGS_H
#define SETTINGS_H

#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <MIDI.h>

#include "sysex.h"
#include "preset.h"

#define SETTINGS_VERSION    4
#define SETTINGS_OFFSET     0x03
#define PRESET_OFFSET       0x10
#define PRESET_SIZE         0x10
#define PRESET_COUNT        15

typedef struct _SettingsData {
    uint8_t midiChannel;
    bool midiThru;
    uint8_t brightness;
    uint8_t argbCount;
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
    .dmxCount = 1,
    .dmxChannelSize = 3,
    .dmxChannelOffset = 1,
    .dmxBrightness = 255,
    .dmxBrightnessChannel = 0,
    .preset = 0
};

class Settings {

private:
    uint8_t read_version();
    bool verify();
    bool read_data();
    void write_settings();
    void write_preset(uint8_t i);
    void write_data();
    void reset();

public:
    SettingsData data;
    PresetData presets[PRESET_COUNT];

    Settings();

    void datacpy(void * destination, bool adjust = false);
    void presetcpy(void * destination, uint8_t i, bool adjust = false);

    void datawrite(void * source, bool adjust = false);
    void presetwrite(void * source, uint8_t i, bool adjust = false);

};

#endif
