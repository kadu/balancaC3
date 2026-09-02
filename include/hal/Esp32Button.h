#pragma once

#include "hal/IButton.h"
#include <OneButton.h>

namespace hal {

class Esp32Button final : public IButton {
public:
    explicit Esp32Button(uint8_t pin, bool activeLow = true, bool pullupActive = true);

    void begin() override;
    void tick() override;

    void setTimings(uint16_t debounceMs, uint16_t clickMs, uint16_t longPressMs) override;
    void ignoreFirstMs(uint16_t ms) override;

    void onDown(ButtonCallback cb) override;
    void onPress(ButtonCallback cb) override;
    void onLongPress(ButtonCallback cb) override;
    void onDoubleClick(ButtonCallback cb) override;

    // Public to allow static trampolines in .cpp to access callbacks
    ButtonCallback _downCallback;
    ButtonCallback _pressCallback;
    ButtonCallback _longPressCallback;
    ButtonCallback _doubleClickCallback;

private:
    OneButton _btn;
    uint32_t  _readyAt = 0;      // millis a partir do qual tick() vale
    bool      _ready   = true;   // sem ignoreFirstMs(), ja nasce pronto
};

} // namespace hal
