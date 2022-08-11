#ifndef RGBW_H
#define RGBW_H

#include "controller.h"

template<uint8_t R_PIN, uint8_t G_PIN, uint8_t B_PIN, uint8_t W_PIN>
class RgbwController : public Controller {

public:
    RgbwController() {
        pinMode(R_PIN, OUTPUT);
        analogWrite(R_PIN, 0);
        pinMode(G_PIN, OUTPUT);
        analogWrite(G_PIN, 0);
        pinMode(B_PIN, OUTPUT);
        analogWrite(B_PIN, 0);
        pinMode(W_PIN, OUTPUT);
        analogWrite(W_PIN, 0);
    };
    void show() {
        if (!this->valid()) return;
        this->_callback(&_c, 0, 1);
        this->sendPixel(0);
    };
    bool valid() {
        return this->_callback != NULL;
    };

private:
    inline void sendPixel(uint16_t index) {
        if (index > 0) return;
        analogWrite(R_PIN, this->_c.r);
        analogWrite(G_PIN, this->_c.g);
        analogWrite(B_PIN, this->_c.b);
        analogWrite(W_PIN, this->getW());
    };
    uint8_t getW() {
        return min(this->_c.r, min(this->_c.g, this->_c.b));
    };

};

#endif
