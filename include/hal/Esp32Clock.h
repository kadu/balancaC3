#pragma once

#include "hal/IClock.h"

namespace hal {

class Esp32Clock final : public IClock {
public:
    uint32_t millis() override;
};

} // namespace hal
