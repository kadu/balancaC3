#pragma once

#include <cstdint>

namespace hal {

class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual void begin() = 0;
    virtual void clear() = 0;
    virtual void show() = 0;

    virtual void drawString(uint8_t x, uint8_t y, const char* text) = 0;
    virtual void drawStringAt(int16_t x, uint8_t y, const char* text) = 0;
    virtual void drawStringCenter(uint8_t y, const char* text) = 0;
    virtual uint8_t stringWidth(const char* text) = 0;
    virtual void setFontSmall() = 0;
    virtual void setFontMedium() = 0;
    virtual void setFontLarge() = 0;
    virtual void setFontXLarge() = 0;
    virtual void drawHLine(uint8_t x, uint8_t y, uint8_t width) = 0;
    virtual void drawBitmap(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t* bitmap) = 0;
    virtual uint8_t displayWidth() = 0;
};

} // namespace hal
