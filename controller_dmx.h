#ifndef DMX_H
#define DMX_H

#include <DmxSimple.h>
#include "controller.h"
#include "settings.h"

template<uint8_t PIN, EOrder RGB_ORDER = RGB>
class DmxController : public Controller {

public:
    DmxController() {
        DmxSimple.usePin(PIN);
        this->configure();
    };
    void configure(uint8_t channelSize = 3, uint8_t channelOffset = 1, uint8_t brightness = 255, uint8_t brightnessChannel = 0) {
        this->_channelSize = channelSize;
        this->_channelOffset = channelOffset;
        this->_brightness = brightness;
        this->_brightnessChannel = brightnessChannel;
        DmxSimple.maxChannel(this->_count * this->_channelSize);
    };
    void configure(SettingsData *settings) {
        this->configure(settings->dmxChannelSize, settings->dmxChannelOffset, settings->dmxBrightness, settings->dmxBrightnessChannel);
    };
    void show() {
        if (!this->valid()) return;
        for (uint16_t i = 0; i < this->_count; i++) {
            this->_callback(&this->_c, i, this->_count);
            this->sendBrightness(i, this->_brightness);
            this->sendPixel(i);
        }
    };

protected:
    uint8_t _channelSize;
    uint8_t _channelOffset;
    uint8_t _brightness;
    uint8_t _brightnessChannel;

    uint16_t getChannel(uint16_t index) {
        index * max(this->_channelSize, 3) + max(this->_channelOffset, 1);
    };

    inline void sendByte(uint16_t channel, uint8_t byteVal) {
        DmxSimple.write(channel, byteVal);
    };
    inline void sendPixel(uint16_t index) {
        uint16_t channel = this->getChannel(index);
        this->sendByte(channel++, this->_c[RGB_ORDER >> 8 & 0x3]);
        this->sendByte(channel++, this->_c[RGB_ORDER >> 4 & 0x3]);
        this->sendByte(channel, this->_c[RGB_ORDER & 0x3]);
    };
    inline void sendBrightness(uint16_t index, uint8_t brightness) {
        if (this->_brightnessChannel == 0) return;
        this->sendByte(index * max(this->_channelSize, 3) + this->_brightnessChannel, this->_brightness);
    };

};

#endif
