#pragma once

#include "hal/IDisplay.h"
#include <U8g2lib.h>

namespace hal {

class Esp32Display final : public IDisplay {
public:
    void begin() override;
    void clear() override;
    void show() override;

    void drawString(uint8_t x, uint8_t y, const char* text) override;
    void drawStringCenter(uint8_t y, const char* text) override;
    void setFontSmall() override;
    void setFontMedium() override;
    void setFontLarge() override;
    void drawHLine(uint8_t x, uint8_t y, uint8_t width) override;

private:
    // SSD1306 128x64 I2C — full buffer mode for simplicity
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C _u8g2{
        U8G2_R0,
        /* scl */ 7,
        /* sda */ 6,
        /* reset */ U8X8_PIN_NONE
    };
};

} // namespace hal
