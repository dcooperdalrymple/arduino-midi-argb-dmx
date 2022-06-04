#include "settings.h"

Settings::Settings() {
    memcpy_P(&_data, &default_settings, sizeof(SettingsData));

    _presets = new PresetData[PRESET_COUNT];
    for (uint8_t i = 0; i < PRESET_COUNT; i++) {
        memcpy_P(&_presets[i], &default_preset, sizeof(PresetData));
    }

    if (!verify()) reset();
    read_data();
}

uint8_t Settings::getMidiChannel() {
    return _data.midiChannel;
}
bool Settings::getMidiThru() {
    return _data.midiThru;
}

uint8_t Settings::getBrightness() {
    return _data.brightness;
}

uint8_t Settings::getArgbCount() {
    return max(_data.argbCount, 1);
}
FastLedType Settings::getArgbType() {
    return (FastLedType)_data.argbType;
}

uint8_t Settings::getDmxCount() {
    return max(_data.dmxCount, 1);
}
uint8_t Settings::getDmxChannelSize() {
    return max(_data.dmxChannelSize, 3);
}
uint8_t Settings::getDmxChannelOffset() {
    return max(_data.dmxChannelOffset, 1);
}
uint8_t Settings::getDmxBrightness() {
    return _data.dmxBrightness;
}
uint8_t Settings::getDmxBrightnessChannel() {
    return _data.dmxBrightnessChannel;
}

uint8_t Settings::getPreset() {
    if (_data.preset >= PRESET_COUNT) return 0;
    return _data.preset;
}
PresetData* Settings::getPresetData(uint8_t i) {
    if (i >= PRESET_COUNT) i = 0;
    return &_presets[i];
}

uint8_t Settings::read_version() {
    if (EEPROM.read(0x00) != MANUFACTURER_ID) return 0;
    if (EEPROM.read(0x01) != DEVICE_ID) return 0;
    return EEPROM.read(0x02);
}

bool Settings::verify() {
    if (read_version() == SETTINGS_VERSION) return true;
    return false;
}

bool Settings::read_data() {
    if (!verify()) return false;
    EEPROM.get(SETTINGS_OFFSET, _data);
    for (uint8_t i = 0; i < PRESET_COUNT; i++) {
        EEPROM.get(PRESET_OFFSET + PRESET_SIZE*i, _presets[i]);
    }
    return true;
}

void Settings::write_settings() {
    EEPROM.put(SETTINGS_OFFSET, _data);
}
void Settings::write_preset(uint8_t i) {
    EEPROM.put(PRESET_OFFSET + PRESET_SIZE*i, _presets[i]);
}
void Settings::write_data() {
    write_settings();
    for (uint8_t i = 0; i < PRESET_COUNT; i++) {
        write_preset(i);
    }
}

void Settings::reset() {
    // Clear EEPROM
    for (int i = 0; i < 256; i++) {
        EEPROM.write(i, 0);
    }

    // Identifier & Version
    EEPROM.write(0x00, MANUFACTURER_ID);
    EEPROM.write(0x01, DEVICE_ID);
    EEPROM.write(0x02, SETTINGS_VERSION);

    write_data();
}

void Settings::datacpy(void * destination, bool adjust = false) {
    memcpy(destination, &_data, sizeof(SettingsData));
    if (adjust) {
        SettingsData *ptr = reinterpret_cast<SettingsData *>(destination);
        ptr->dmxBrightness /= 2;
    }
}
void Settings::presetcpy(void * destination, uint8_t i, bool adjust = false) {
    if (i >= PRESET_COUNT) return;
    memcpy(destination, &_presets[i], sizeof(PresetData));
    if (adjust) {
        PresetData *ptr = reinterpret_cast<PresetData *>(destination);
        ptr->color_rgb.r /= 2;
        ptr->color_rgb.g /= 2;
        ptr->color_rgb.b /= 2;
        ptr->color_hsv.h /= 2;
        ptr->color_hsv.s /= 2;
        ptr->color_hsv.v /= 2;
    }
}

void Settings::datawrite(void * source, bool adjust = false) {
    memcpy(&_data, source, sizeof(SettingsData));
    if (adjust) {
        _data.dmxBrightness *= 2;
    }
    write_settings();
}
void Settings::presetwrite(void * source, uint8_t i, bool adjust = false) {
    if (i >= PRESET_COUNT) return;
    memcpy(&_presets[i], source, sizeof(PresetData));
    if (adjust) {
        _presets[i].color_rgb.r *= 2;
        _presets[i].color_rgb.g *= 2;
        _presets[i].color_rgb.b *= 2;
        _presets[i].color_hsv.h *= 2;
        _presets[i].color_hsv.s *= 2;
        _presets[i].color_hsv.v *= 2;
    }
    write_preset(i);
}
