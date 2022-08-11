#ifndef ARGB_H
#define ARGB_H

#include <digitalWriteFast.h>
#include "controller.h"

// https://github.com/bigjosh/SimpleNeoPixelDemo/

#define NS_PER_SEC (1000000000L)
#define CYCLES_PER_SEC (F_CPU)
#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )
#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )

#define T1H_ADJ (50)
#define T1L_ADJ (-92)
#define T0H_ADJ (-17)
#define T0L_ADJ (-67)

/**
 * T1H = Width of a 1 bit in ns
 * T1L = Width of a 1 bit in ns
 * T0H = Width of a 0 bit in ns
 * T0L = Width of a 0 bit in ns
 * RES = Width of a low gap between bits to cause a frame to latch
 */
template<unsigned T1H, unsigned T1L, unsigned T0H, unsigned T0L, unsigned RES, uint8_t PIN, EOrder RGB_ORDER = RGB>
class ArgbController : public Controller {

public:
    ArgbController() {
        pinModeFast(PIN, OUTPUT);
    };
    void show(bool wait = true) {
        if (!this->valid()) return;
        cli();
        for (uint16_t i = 0; i < this->_count; i++) {
            this->_callback(&this->_c, i, this->_count);
            this->sendPixel();
        }
        sei();
        if (wait) _delay_us((RES / 1000UL) + 1); // Wait for pixels to latch and display
    };

private:
    inline void sendByte(uint8_t value) {
        asm volatile (
            "argb_bit:              \n\t"
            "dec %[bits]            \n\t"   // Decrement bit counter (1 cycle)
            "brmi argb_ret          \n\t"   // Check if we're at the final bit (1 cycle)
            "lsl %[byteVal]         \n\t"   // Shift byte left (1 cycle)
            "sbi %[port], %[bit]    \n\t"   // Set the output bit (doesn't affect carry flag) (2 cycles)
            "brcc argb_t0           \n"     // If bit was 0 (msb is moved into carry) (1 cycle if t1, 2 cycles if t0)
            "argb_t1:               \n\t"
            ".rept %[t1h]           \n\t"   // Delay T1H
            "nop                    \n\t"
            ".endr                  \n\t"
            "cbi %[port], %[bit]    \n\t"   // Clear the output bit
            /*
            ".rept %[t1l]           \n\t"   // Delay T1L
            "nop                    \n\t"
            ".endr                  \n\t"
            */
            "jmp argb_bit           \n"     // Next bit (3 cycles)
            "argb_t0:               \n\t"   // Delay T0H
            ".rept %[t0h]           \n\t"
            "nop                    \n\t"
            ".endr                  \n\t"
            "cbi %[port], %[bit]    \n\t"   // Clear the output bit
            ".rept %[t0l]           \n\t"   // Delay T0L
            "nop                    \n\t"
            ".endr                  \n\t"
            "jmp argb_bit           \n"     // Next bit (3 cycles)
            "argb_ret:              \n\t"
            "nop                    \n\t"
            ::
            [port]      "I"   (_SFR_IO_ADDR(*__digitalPinToPortReg(PIN))),
            [bit]       "I"   (__digitalPinToBit(PIN)),
            [t1h]       "I"   (NS_TO_CYCLES(T1H+T1H_ADJ) - 3),
            //[t1l]       "I"   (NS_TO_CYCLES(T1L+T1L_ADJ) - 6),
            [t0h]       "I"   (NS_TO_CYCLES(T0H+T0H_ADJ) - 4),
            [t0l]       "I"   (NS_TO_CYCLES(T0L+T0L_ADJ) - 6),
            [byteVal]   "r"   (value),
            [bits]      "r"   (8)
        );
    };
    inline void sendPixel() {
        this->sendByte(this->_c[RGB_ORDER >> 8 & 0x03]);
        this->sendByte(this->_c[RGB_ORDER >> 4 & 0x03]);
        this->sendByte(this->_c[RGB_ORDER & 0x03]);
    };

};

template<uint8_t PIN, EOrder RGB_ORDER>
class WS2812B : public ArgbController<800, 450, 400, 850, 50000, PIN, RGB_ORDER> {};

#endif
