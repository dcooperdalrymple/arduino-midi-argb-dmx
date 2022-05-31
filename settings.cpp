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
    return _data.argbCount;
}
FastLedType Settings::getArgbType() {
    return (FastLedType)_data.argbType;
}

uint8_t Settings::getDmxCount() {
    return _data.dmxCount;
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

void Settings::write_data() {
    EEPROM.put(SETTINGS_OFFSET, _data);
    for (uint8_t i = 0; i < PRESET_COUNT; i++) {
        EEPROM.put(PRESET_OFFSET + PRESET_SIZE*i, _presets[i]);
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

bool Settings::handleSysex(byte* data, unsigned size) {
    if (size != 256) return false;

    if (data[0x00] != MANUFACTURER_ID) return false;
    if (data[0x01] != DEVICE_ID) return false;
    if (data[0x02] != SETTINGS_VERSION) return false;

    for (unsigned i = 0; i < size; i++) {
        EEPROM.write(i, data[i]);
    }

    read_data();
    return true;
}
