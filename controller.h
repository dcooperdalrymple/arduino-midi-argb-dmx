#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "color.h"

typedef void (*RgbCallback)(CRGB *c, uint8_t index, uint16_t count);

class Controller {

public:
    Controller() { };
    void setCallback(RgbCallback cb) {
        this->_callback = cb;
    };
    void setCount(uint16_t cnt) {
        this->_count = cnt;
    };
    void show() { };

protected:
    RgbCallback _callback = NULL;
    uint16_t _count = 0;
    CRGB _c = CRGB::Black;

    bool valid() {
        return this->_count > 0 && this->_callback != NULL;
    };

    inline void sendByte(uint8_t byteVal) { };
    inline void sendPixel() { };

};

#endif
