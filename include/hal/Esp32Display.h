#pragma once

#include "hal/IDisplay.h"
#include <U8g2lib.h>
#include <Wire.h>

namespace hal {

class Esp32Display final : public IDisplay {
public:
    void begin() override;
    void clear() override;
    void show() override;

    void    drawString(uint8_t x, uint8_t y, const char* text) override;
    void    drawStringAt(int16_t x, uint8_t y, const char* text) override;
    void    drawStringCenter(uint8_t y, const char* text) override;
    uint8_t stringWidth(const char* text) override;
    void    setFontSmall() override;
    void    setFontMedium() override;
    void    setFontLarge() override;
    void    setFontXLarge() override;
    void    drawHLine(uint8_t x, uint8_t y, uint8_t width) override;
    void    drawVLine(uint8_t x, uint8_t y, uint8_t height) override;
    void    drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h) override;
    void    drawTriangle(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) override;
    void    drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap) override;
    void    drawBitmapMSB(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap) override;
    uint8_t displayWidth() override;

private:
    // SSD1306 128x64 — hardware I2C, shares Wire bus with NAU7802
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2{U8G2_R0, U8X8_PIN_NONE};
};

} // namespace hal
