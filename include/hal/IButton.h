#pragma once

#include <functional>

namespace hal {

using ButtonCallback = std::function<void()>;

class IButton {
public:
    virtual ~IButton() = default;

    virtual void begin() = 0;
    virtual void tick() = 0;

    virtual void onDown(ButtonCallback cb) = 0;
    virtual void onPress(ButtonCallback cb) = 0;
    virtual void onLongPress(ButtonCallback cb) = 0;
    virtual void onDoubleClick(ButtonCallback cb) = 0;
};

} // namespace hal
