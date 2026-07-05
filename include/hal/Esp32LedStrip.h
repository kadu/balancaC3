#pragma once

#include "hal/ILedStrip.h"
#include "config.h"
#include <FastLED.h>

namespace hal {

class Esp32LedStrip final : public ILedStrip {
public:
    void begin() override;
    void setBrightness(uint8_t brightness) override;
    void setAll(Color color) override;
    void setOne(uint8_t index, Color color) override;
    void clear() override;
    void show() override;
    uint8_t count() override;

private:
    CRGB _leds[LED_COUNT];
};

} // namespace hal
