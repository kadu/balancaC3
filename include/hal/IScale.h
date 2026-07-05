#pragma once

#include <cstdint>
#include <cstddef>

namespace hal {

class IScale {
public:
    virtual ~IScale() = default;

    virtual bool begin() = 0;
    virtual bool available() = 0;

    // Raw ADC reading (averaged over N samples)
    virtual int32_t readRaw(uint8_t samples = 1) = 0;

    // Calibration helpers
    virtual void setZeroOffset(int32_t offset) = 0;
    virtual void setScaleFactor(float factor) = 0;
    virtual int32_t zeroOffset() = 0;
    virtual float   scaleFactor() = 0;

    // Tare: capture current raw as zero
    virtual int32_t tare(uint8_t samples = 10) = 0;
};

} // namespace hal
