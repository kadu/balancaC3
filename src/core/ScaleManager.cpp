#include "core/ScaleManager.h"
#include "events/EventType.h"
#include "config.h"
#include <cstdio>
#include <cstdlib>

namespace core {

ScaleManager::ScaleManager(hal::IScale& scale, hal::IStorage& storage,
                            hal::IClock& clock, events::EventBus& eventBus)
    : _scale(scale), _storage(storage), _clock(clock), _eventBus(eventBus) {}

void ScaleManager::begin() {
    _ready = _scale.begin();
    if (!_ready) {
        _eventBus.publish({events::EventType::ScaleNotFound});
        return;
    }
    loadCalibration();
    loadFilterConfig();
    if (_calibrated) _eventBus.publish({events::EventType::ScaleCalibrated});
    _eventBus.subscribe(events::EventType::Button2Pressed, this);
}

void ScaleManager::loop() {
    if (!_ready) return;
    if (!_scale.available()) return;

    _accumulator += _scale.readRaw(1);
    _sampleCount++;

    if (_sampleCount < _filter.samples) return;

    int32_t raw  = static_cast<int32_t>(_accumulator / _sampleCount);
    _accumulator = 0;
    _sampleCount = 0;
    _lastRaw     = raw;

    float grams = _calibrated
        ? static_cast<float>(raw - _scale.zeroOffset()) / _scale.scaleFactor()
        : static_cast<float>(raw);

    if (!_emaInitialized) { _emaWeight = grams; _emaInitialized = true; }
    _emaWeight = _filter.emaAlpha * grams + (1.0f - _filter.emaAlpha) * _emaWeight;

    float displayed = _emaWeight;
    if (_calibrated && displayed > -_filter.zeroSnapG && displayed < _filter.zeroSnapG)
        displayed = 0.0f;

    float delta = displayed - _lastWeight;
    if (delta < 0.0f) delta = -delta;
    if (delta >= _filter.deadbandG || !_calibrated) {
        _lastWeight = displayed;
        publishWeight(displayed);
    }
}

void ScaleManager::onEvent(const events::Event& event) {
    if (event.type == events::EventType::Button2Pressed) commandTare();
}

void ScaleManager::commandTare() {
    if (!_ready) return;
    _scale.tare(SCALE_CALIBRATION_SAMPLES);
    _emaWeight      = 0.0f;
    _lastWeight     = 0.0f;
    _emaInitialized = false;
    saveCalibration();
    _eventBus.publish({events::EventType::ScaleTared});
    publishWeight(0.0f);
}

void ScaleManager::commandCalibrateStep1() {
    if (!_ready) return;
    _rawStep1 = _scale.readRaw(SCALE_CALIBRATION_SAMPLES);
}

void ScaleManager::commandCalibrateStep2(float knownGrams) {
    if (!_ready || knownGrams <= 0.0f) return;
    int32_t rawKnown = _scale.readRaw(SCALE_CALIBRATION_SAMPLES);
    int32_t rawZero  = _scale.zeroOffset();
    float   factor   = static_cast<float>(rawKnown - rawZero) / knownGrams;
    _scale.setScaleFactor(factor);
    _calibrated = true;
    saveCalibration();
    _eventBus.publish({events::EventType::ScaleCalibrated});
}

void ScaleManager::setFilterConfig(const ScaleFilterConfig& cfg, bool persist) {
    _filter          = cfg;
    _emaInitialized  = false; // reset EMA with new alpha
    if (persist) saveFilterConfig();
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

void ScaleManager::loadFilterConfig() {
    char buf[16] = {};
    if (_storage.getString(STORAGE_KEY_SCALE_EMA,      buf, sizeof(buf))) _filter.emaAlpha  = atof(buf);
    if (_storage.getString(STORAGE_KEY_SCALE_DEADBAND, buf, sizeof(buf))) _filter.deadbandG = atof(buf);
    if (_storage.getString(STORAGE_KEY_SCALE_SNAP,     buf, sizeof(buf))) _filter.zeroSnapG = atof(buf);
    if (_storage.getString(STORAGE_KEY_SCALE_SAMPLES,  buf, sizeof(buf))) {
        int s = atoi(buf);
        if (s >= 1 && s <= 20) _filter.samples = static_cast<uint8_t>(s);
    }
}

void ScaleManager::saveFilterConfig() {
    char buf[16] = {};
    snprintf(buf, sizeof(buf), "%.3f", _filter.emaAlpha);
    _storage.putString(STORAGE_KEY_SCALE_EMA, buf);
    snprintf(buf, sizeof(buf), "%.2f", _filter.deadbandG);
    _storage.putString(STORAGE_KEY_SCALE_DEADBAND, buf);
    snprintf(buf, sizeof(buf), "%.2f", _filter.zeroSnapG);
    _storage.putString(STORAGE_KEY_SCALE_SNAP, buf);
    snprintf(buf, sizeof(buf), "%u", _filter.samples);
    _storage.putString(STORAGE_KEY_SCALE_SAMPLES, buf);
}

void ScaleManager::publishWeight(float grams) {
    _payload = {grams, _calibrated};
    _eventBus.publish({events::EventType::WeightUpdated, &_payload});
}

} // namespace core
