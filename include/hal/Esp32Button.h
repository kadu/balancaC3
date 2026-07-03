#pragma once

#include "hal/IButton.h"
#include <OneButton.h>

namespace hal {

class Esp32Button final : public IButton {
public:
    explicit Esp32Button(uint8_t pin, bool activeLow = true, bool pullupActive = true);

    void begin() override;
    void tick() override;

    void onPress(ButtonCallback cb) override;
    void onLongPress(ButtonCallback cb) override;
    void onDoubleClick(ButtonCallback cb) override;

    // Public to allow static trampolines in .cpp to access callbacks
    ButtonCallback _pressCallback;
    ButtonCallback _longPressCallback;
    ButtonCallback _doubleClickCallback;

private:
    OneButton _btn;
};

} // namespace hal
