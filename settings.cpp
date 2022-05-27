#include "settings.h"

Settings::Settings() : _data(default_settings) {
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
    if (EEPROM.read(0x00) != SETTINGS_ID) return 0;
    return EEPROM.read(0x01);
}

bool Settings::verify() {
    if (read_version() == SETTINGS_VERSION) return true;
    return false;
}

bool Settings::read_data() {
    if (!verify()) return false;
    EEPROM.get(SETTINGS_OFFSET, _data);
    return true;
}

void Settings::write_data() {
    EEPROM.put(SETTINGS_OFFSET, _data);
}

void Settings::reset() {
    // Clear EEPROM
    for (int i = 0; i < 256; i++) {
        EEPROM.write(i, 0);
    }

    // Identifier & Version
    EEPROM.write(0x00, SETTINGS_ID);
    EEPROM.write(0x01, SETTINGS_VERSION);

    write_data();
}
