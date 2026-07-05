#include "hal/Esp32Display.h"
#include "config.h"
#include <Wire.h>

namespace hal {

void Esp32Display::begin() {
    _u8g2.begin();
}

void Esp32Display::clear() {
    _u8g2.clearBuffer();
}

void Esp32Display::show() {
    _u8g2.sendBuffer();
}

void Esp32Display::drawString(uint8_t x, uint8_t y, const char* text) {
    _u8g2.drawStr(x, y, text);
}

void Esp32Display::drawStringAt(int16_t x, uint8_t y, const char* text) {
    _u8g2.drawStr(x, y, text);
}

void Esp32Display::drawStringCenter(uint8_t y, const char* text) {
    uint8_t w = _u8g2.getStrWidth(text);
    uint8_t x = (128 - w) / 2;
    _u8g2.drawStr(x, y, text);
}

uint8_t Esp32Display::stringWidth(const char* text) {
    return static_cast<uint8_t>(_u8g2.getStrWidth(text));
}

void Esp32Display::setFontSmall() {
    _u8g2.setFont(u8g2_font_6x10_tf);
}

void Esp32Display::setFontMedium() {
    _u8g2.setFont(u8g2_font_9x15_tf);
}

void Esp32Display::setFontLarge() {
    _u8g2.setFont(u8g2_font_10x20_tf);
}

void Esp32Display::setFontXLarge() {
    _u8g2.setFont(u8g2_font_logisoso28_tf);
}

void Esp32Display::drawHLine(uint8_t x, uint8_t y, uint8_t width) {
    _u8g2.drawHLine(x, y, width);
}

uint8_t Esp32Display::displayWidth() {
    return 128;
}

} // namespace hal
