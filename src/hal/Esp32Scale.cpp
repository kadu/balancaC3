#include "hal/Esp32Scale.h"

namespace hal {

bool Esp32Scale::begin() {
    if (!_nau.begin()) return false;
    _nau.setSampleRate(NAU7802_SPS_80);    // 80 SPS — fast enough for live coffee weighing
    _nau.setGain(NAU7802_GAIN_128);
    _nau.calibrateAFE();                   // internal offset/gain calibration
    return true;
}

bool Esp32Scale::available() {
    return _nau.available();
}

int32_t Esp32Scale::readRaw(uint8_t samples) {
    // For samples > 1 we allow a brief blocking wait only during calibration/tare
    // (called from user action, not from loop). Normal loop() passes samples=1.
    if (samples <= 1) return _nau.getReading();
    int64_t sum = 0;
    for (uint8_t i = 0; i < samples; ++i) {
        uint32_t t = millis();
        while (!_nau.available() && (millis() - t < 500)) delay(1);
        sum += _nau.getReading();
    }
    return static_cast<int32_t>(sum / samples);
}

void Esp32Scale::setZeroOffset(int32_t offset) {
    _nau.setZeroOffset(offset);
}

void Esp32Scale::setScaleFactor(float factor) {
    _nau.setCalibrationFactor(factor);
}

int32_t Esp32Scale::zeroOffset() {
    return _nau.getZeroOffset();
}

float Esp32Scale::scaleFactor() {
    return _nau.getCalibrationFactor();
}

int32_t Esp32Scale::tare(uint8_t samples) {
    int32_t zero = readRaw(samples);
    _nau.setZeroOffset(zero);
    return zero;
}

} // namespace hal
