#pragma once

namespace hal {

class IDevice {
public:
    virtual ~IDevice() = default;
    virtual void restart() = 0;
};

} // namespace hal
