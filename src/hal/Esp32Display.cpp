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

void Esp32Display::drawVLine(uint8_t x, uint8_t y, uint8_t height) {
    _u8g2.drawVLine(x, y, height);
}

void Esp32Display::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    _u8g2.drawBox(x, y, w, h);
}

void Esp32Display::drawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    _u8g2.drawTriangle(x0, y0, x1, y1, x2, y2);
}

void Esp32Display::drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap) {
    _u8g2.drawXBMP(x, y, w, h, bitmap);
}

void Esp32Display::drawBitmapMSB(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap) {
    // Adafruit-format bitmaps are MSB-first; U8g2 drawBitmap is also MSB-first.
    _u8g2.drawBitmap(x, y, (w + 7) / 8, h, bitmap);
}

uint8_t Esp32Display::displayWidth() {
    return 128;
}

} // namespace hal
