#include "hal/Esp32Button.h"
#include <Arduino.h>

namespace hal {

// Static trampolines — OneButton only accepts plain function pointers
static void trampolineDown(void* self) {
    auto* b = static_cast<Esp32Button*>(self);
    if (b->_downCallback) b->_downCallback();
}
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
    // Nao usa millis() < _readyAt direto: a comparacao quebraria no rollover de
    // millis(). Depois que solta, o latch nunca mais e reavaliado.
    if (!_ready) {
        if (millis() < _readyAt) return;
        _ready = true;
    }
    _btn.tick();
}

void Esp32Button::setTimings(uint16_t debounceMs, uint16_t clickMs, uint16_t longPressMs) {
    _btn.setDebounceMs(debounceMs);
    _btn.setClickMs(clickMs);
    _btn.setPressMs(longPressMs);
}

void Esp32Button::ignoreFirstMs(uint16_t ms) {
    _readyAt = millis() + ms;
    _ready   = false;
}

void Esp32Button::onDown(ButtonCallback cb) {
    _downCallback = cb;
    _btn.attachPress(trampolineDown, this);
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
