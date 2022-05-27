#ifndef SETTINGS_H
#define SETTINGS_H

#include <EEPROM.h>
#include <MIDI.h>
#include <FastLED.h>

#define SETTINGS_ID         0xaa
#define SETTINGS_VERSION    1
#define SETTINGS_OFFSET     0x02

enum FastLedType {
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
} SettingsData;

static const SettingsData default_settings = {
    .midiChannel = MIDI_CHANNEL_OMNI,
    .midiThru = true,
    .brightness = 64,
    .argbCount = 12,
    .argbType = FLTYPE_WS2812B,
    .dmxCount = 3
};

class Settings {
    
private:
    SettingsData _data;

    uint8_t read_version();
    bool verify();
    bool read_data();
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

};

#endif
