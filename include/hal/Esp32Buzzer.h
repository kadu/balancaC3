#pragma once

#include "hal/IBuzzer.h"

namespace hal {

class Esp32Buzzer final : public IBuzzer {
public:
    void begin() override;
    void beep(uint32_t frequencyHz, uint32_t durationMs) override;
    void stop() override;
};

} // namespace hal
