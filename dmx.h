#ifndef DMX_H
#define DMX_H

#include <DmxSimple.h>
#include <FastLED.h>

FASTLED_NAMESPACE_BEGIN

template <uint8_t DATA_PIN, EOrder RGB_ORDER = RGB> class DMXSimpleController : public CPixelLEDController<RGB_ORDER> {
public:

    virtual void init() {
        brightness = 255;
        brightnessChannel = 0;

        channelSize = 3;
        channelOffset = 1;

        DmxSimple.usePin(DATA_PIN);
    }

    virtual void setMaxChannel(int channel) {
        DmxSimple.maxChannel(channel);
    }
    virtual void setChannelSize(uint8_t size) {
        channelSize = size;
    }
    virtual void setChannelOffset(uint8_t channel) {
        channelOffset = channel;
    }

    virtual void setBrightness(uint8_t brightness) {
        brightness = brightness;
    }
    virtual void setBrightnessChannel(uint8_t channel) {
        brightnessChannel = channel;
    }

protected:

    uint8_t channelSize;
    uint8_t channelOffset;

    uint8_t brightness;
    uint8_t brightnessChannel;

    virtual void showPixels(PixelController<RGB_ORDER> & pixels) {
        int iPixel = 0;
        int iChannel = 1;
        while (pixels.has(1)) {
            iChannel = iPixel++ * channelSize;
            if (brightnessChannel > 0) {
                DmxSimple.write(iChannel + brightnessChannel, brightness);
            }

            iChannel += channelOffset;
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
