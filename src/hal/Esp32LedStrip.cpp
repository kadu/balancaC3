#include "hal/Esp32LedStrip.h"

namespace hal {

void Esp32LedStrip::begin() {
    FastLED.addLeds<WS2812B, PIN_LED_STRIP, GRB>(_leds, LED_COUNT);
    FastLED.setBrightness(LED_BRIGHTNESS_DEFAULT);
    clear();
    show();
}

void Esp32LedStrip::setBrightness(uint8_t brightness) {
    FastLED.setBrightness(brightness);
}

void Esp32LedStrip::setAll(Color color) {
    for (uint8_t i = 0; i < LED_COUNT; ++i) {
        _leds[i] = CRGB(color.r, color.g, color.b);
    }
}

void Esp32LedStrip::setOne(uint8_t index, Color color) {
    if (index < LED_COUNT) {
        _leds[index] = CRGB(color.r, color.g, color.b);
    }
}

void Esp32LedStrip::clear() {
    FastLED.clear();
}

void Esp32LedStrip::show() {
    FastLED.show();
}

uint8_t Esp32LedStrip::count() {
    return LED_COUNT;
}

} // namespace hal
