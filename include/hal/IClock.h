#pragma once

#include <cstdint>

namespace hal {

class IClock {
public:
    virtual ~IClock() = default;
    virtual uint32_t millis() = 0;
};

} // namespace hal
