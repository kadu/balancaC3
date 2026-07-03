#include "hal/Esp32Button.h"

namespace hal {

// Static trampolines — OneButton only accepts plain function pointers
static void trampolinePress(void* self) {
    auto* b = static_cast<Esp32Button*>(self);
    if (b->_pressCallback) b->_pressCallback();
}
static void trampolineLong(void* self) {
    auto* b = static_cast<Esp32Button*>(self);
    if (b->_longPressCallback) b->_longPressCallback();
}
static void trampolineDouble(void* self) {
    auto* b = static_cast<Esp32Button*>(self);
    if (b->_doubleClickCallback) b->_doubleClickCallback();
}

Esp32Button::Esp32Button(uint8_t pin, bool activeLow, bool pullupActive)
    : _btn(pin, activeLow, pullupActive) {}

void Esp32Button::begin() {}

void Esp32Button::tick() {
    _btn.tick();
}

void Esp32Button::onPress(ButtonCallback cb) {
    _pressCallback = cb;
    _btn.attachClick(trampolinePress, this);
}

void Esp32Button::onLongPress(ButtonCallback cb) {
    _longPressCallback = cb;
    _btn.attachLongPressStart(trampolineLong, this);
}

void Esp32Button::onDoubleClick(ButtonCallback cb) {
    _doubleClickCallback = cb;
    _btn.attachDoubleClick(trampolineDouble, this);
}

} // namespace hal
