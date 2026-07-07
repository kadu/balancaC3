#pragma once

#include <cstdint>

namespace hal {

class IBuzzer {
public:
    virtual ~IBuzzer() = default;

    virtual void begin() = 0;
    virtual void beep(uint32_t frequencyHz, uint32_t durationMs) = 0;
    virtual void stop() = 0;
};

} // namespace hal
