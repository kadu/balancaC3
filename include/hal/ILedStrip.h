#pragma once

#include <cstdint>

namespace hal {

struct Color {
    uint8_t r, g, b;
    static Color white()     { return {255, 255, 255}; }
    static Color black()     { return {0,   0,   0};   }
    static Color green()     { return {0,   255, 0};   }
    static Color yellow()    { return {255, 200, 0};   }
    static Color blue()      { return {0,   0,   255}; }
    static Color lightBlue() { return {0,   180, 255}; }
    static Color darkBlue()  { return {0,   0,   80};  }
    static Color red()       { return {255, 0,   0};   }
};

class ILedStrip {
public:
    virtual ~ILedStrip() = default;

    virtual void begin() = 0;
    virtual void setBrightness(uint8_t brightness) = 0;
    virtual void setAll(Color color) = 0;
    virtual void setOne(uint8_t index, Color color) = 0;
    virtual void clear() = 0;
    virtual void show() = 0;
    virtual uint8_t count() = 0;
};

} // namespace hal
