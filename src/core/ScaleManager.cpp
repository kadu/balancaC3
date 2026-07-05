#include "core/ScaleManager.h"
#include "events/EventType.h"
#include "config.h"
#include <cstdio>
#include <cstdlib>

// At 10 SPS each sample takes ~100ms. We collect SCALE_AVERAGE_SAMPLES
// one per loop() tick (non-blocking) then publish the average.

namespace core {

ScaleManager::ScaleManager(hal::IScale& scale, hal::IStorage& storage,
                            hal::IClock& clock, events::EventBus& eventBus)
    : _scale(scale), _storage(storage), _clock(clock), _eventBus(eventBus) {}

void ScaleManager::begin() {
    _ready = _scale.begin();
    if (!_ready) return;
    loadCalibration();
}

void ScaleManager::loop() {
    if (!_ready) return;
    if (!_scale.available()) return;

    // Accumulate one raw sample per tick — no blocking delay
    _accumulator += _scale.readRaw(1);
    _sampleCount++;

    if (_sampleCount < SCALE_AVERAGE_SAMPLES) return;

    int32_t raw  = static_cast<int32_t>(_accumulator / _sampleCount);
    _accumulator = 0;
    _sampleCount = 0;
    _lastRaw     = raw;

    float grams = _calibrated
        ? static_cast<float>(raw - _scale.zeroOffset()) / _scale.scaleFactor()
        : 0.0f;

    if (grams > -0.5f && grams < 0.5f) grams = 0.0f;

    if (grams != _lastWeight || raw != _lastRaw) {
        _lastWeight = grams;
        publishWeight(grams);
    }
}

void ScaleManager::onEvent(const events::Event& /*event*/) {}

void ScaleManager::commandTare() {
    if (!_ready) return;
    _scale.tare(SCALE_AVERAGE_SAMPLES);
    saveCalibration();
    _eventBus.publish({events::EventType::ScaleTared});
    publishWeight(0.0f);
}

void ScaleManager::commandCalibrateStep1() {
    if (!_ready) return;
    _rawStep1 = _scale.readRaw(SCALE_AVERAGE_SAMPLES);
}

void ScaleManager::commandCalibrateStep2(float knownGrams) {
    if (!_ready || knownGrams <= 0.0f) return;

    int32_t rawKnown = _scale.readRaw(SCALE_AVERAGE_SAMPLES);
    int32_t rawZero  = _scale.zeroOffset();
    float   factor   = static_cast<float>(rawKnown - rawZero) / knownGrams;

    _scale.setScaleFactor(factor);
    _calibrated = true;
    saveCalibration();
    _eventBus.publish({events::EventType::ScaleCalibrated});
}

void ScaleManager::loadCalibration() {
    char buf[32] = {};

    bool hasZero   = _storage.getString(STORAGE_KEY_SCALE_ZERO,   buf, sizeof(buf));
    int32_t zero   = hasZero ? static_cast<int32_t>(atol(buf)) : 0;

    bool hasFactor = _storage.getString(STORAGE_KEY_SCALE_FACTOR, buf, sizeof(buf));
    float factor   = hasFactor ? atof(buf) : 0.0f;

    if (hasZero && hasFactor && factor != 0.0f) {
        _scale.setZeroOffset(zero);
        _scale.setScaleFactor(factor);
        _calibrated = true;
    }
}

void ScaleManager::saveCalibration() {
    char buf[32] = {};
    snprintf(buf, sizeof(buf), "%ld", (long)_scale.zeroOffset());
    _storage.putString(STORAGE_KEY_SCALE_ZERO, buf);
    snprintf(buf, sizeof(buf), "%.6f", _scale.scaleFactor());
    _storage.putString(STORAGE_KEY_SCALE_FACTOR, buf);
}

void ScaleManager::publishWeight(float grams) {
    _payload = {grams, _calibrated};
    _eventBus.publish({events::EventType::WeightUpdated, &_payload});
}

} // namespace core
