#ifndef DMX_H
#define DMX_H

#include <DmxSimple.h>
#include <FastLED.h>

#include "settings.h"

FASTLED_NAMESPACE_BEGIN

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB> class DMXSimpleController : public CPixelLEDController<RGB_ORDER> {
public:

    virtual void init() {
        DmxSimple.usePin(DATA_PIN);
    }

    virtual void assign(Settings* ptr) {
        settings = ptr;
    }

    virtual void update() {
        DmxSimple.maxChannel(settings->data.dmxCount * settings->data.dmxChannelSize);
    }

protected:

    Settings* settings;

    virtual void showPixels(PixelController<RGB_ORDER> & pixels) {
        int iPixel = 0;
        int iChannel = 1;
        while (pixels.has(1)) {
            iChannel = iPixel++ * max(settings->data.dmxChannelSize, 3);
            if (settings->data.dmxBrightnessChannel > 0) {
                DmxSimple.write(iChannel + settings->data.dmxBrightnessChannel, settings->data.dmxBrightness);
            }

            iChannel += max(settings->data.dmxChannelOffset, 1);
            DmxSimple.write(iChannel++, pixels.loadAndScale0());
            DmxSimple.write(iChannel++, pixels.loadAndScale1());
            DmxSimple.write(iChannel++, pixels.loadAndScale2());

            pixels.advanceData();
            pixels.stepDithering();
        }
    }

};

template<uint8_t DATA_PIN, EOrder RGB_ORDER> class DMXSIMPLE : public DMXSimpleController<DATA_PIN, RGB_ORDER> {};

FASTLED_NAMESPACE_END

#endif
