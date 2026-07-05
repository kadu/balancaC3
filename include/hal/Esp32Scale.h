#pragma once

#include "hal/IScale.h"
#include <SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h>

namespace hal {

class Esp32Scale final : public IScale {
public:
    bool begin() override;
    bool available() override;

    int32_t readRaw(uint8_t samples = 1) override;

    void    setZeroOffset(int32_t offset) override;
    void    setScaleFactor(float factor) override;
    int32_t zeroOffset() override;
    float   scaleFactor() override;

    int32_t tare(uint8_t samples = 10) override;

private:
    NAU7802 _nau;
};

} // namespace hal
