#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IScale.h"
#include "hal/IStorage.h"
#include "hal/IClock.h"
#include "config.h"

namespace core {

struct WeightPayload {
    float grams;
    bool  calibrated;
};

struct ScaleFilterConfig {
    float   emaAlpha   = SCALE_EMA_ALPHA;
    float   deadbandG  = SCALE_DEADBAND_G;
    float   zeroSnapG  = SCALE_ZERO_SNAP_G;
    uint8_t samples    = SCALE_AVERAGE_SAMPLES;
};

class ScaleManager final : public events::IEventHandler {
public:
    ScaleManager(hal::IScale& scale, hal::IStorage& storage,
                 hal::IClock& clock, events::EventBus& eventBus);

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

    void commandTare();
    void commandCalibrateStep1();
    void commandCalibrateStep2(float knownGrams);

    float   lastWeight()   const { return _lastWeight; }
    int32_t lastRaw()      const { return _lastRaw; }
    bool    isCalibrated() const { return _calibrated; }
    bool    isReady()      const { return _ready; }

    ScaleFilterConfig filterConfig() const { return _filter; }
    void setFilterConfig(const ScaleFilterConfig& cfg, bool persist = true);

private:
    hal::IScale&      _scale;
    hal::IStorage&    _storage;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;

    bool     _ready          = false;
    bool     _calibrated     = false;
    float    _lastWeight     = 0.0f;
    float    _emaWeight      = 0.0f;
    bool     _emaInitialized = false;
    int32_t  _lastRaw        = 0;
    int64_t  _accumulator    = 0;
    uint8_t  _sampleCount    = 0;
    int32_t  _rawStep1       = 0;

    ScaleFilterConfig _filter{};
    WeightPayload     _payload{};

    void loadCalibration();
    void saveCalibration();
    void loadFilterConfig();
    void saveFilterConfig();
    void publishWeight(float grams);
};

} // namespace core
