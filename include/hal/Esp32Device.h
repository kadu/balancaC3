#pragma once

#include "hal/IDevice.h"

namespace hal {

class Esp32Device final : public IDevice {
public:
    void restart() override;
};

} // namespace hal
