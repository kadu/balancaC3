#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IScale.h"
#include "hal/IStorage.h"
#include "hal/IClock.h"

namespace core {

struct WeightPayload {
    float grams;
    bool  calibrated;
};

class ScaleManager final : public events::IEventHandler {
public:
    ScaleManager(hal::IScale& scale, hal::IStorage& storage,
                 hal::IClock& clock, events::EventBus& eventBus);

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

    // Called from WebApp handlers
    void commandTare();
    void commandCalibrateStep1();                   // capture raw with known weight on
    void commandCalibrateStep2(float knownGrams);   // compute and save factor
    float   lastWeight()  const { return _lastWeight; }
    int32_t lastRaw()     const { return _lastRaw; }
    bool    isCalibrated() const { return _calibrated; }
    bool    isReady()      const { return _ready; }

private:
    hal::IScale&      _scale;
    hal::IStorage&    _storage;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;

    bool     _ready        = false;
    bool     _calibrated   = false;
    float    _lastWeight   = 0.0f;
    int32_t  _lastRaw      = 0;

    // Non-blocking accumulator for averaging
    int64_t  _accumulator  = 0;
    uint8_t  _sampleCount  = 0;

    int32_t  _rawStep1     = 0;

    WeightPayload _payload{};

    void loadCalibration();
    void saveCalibration();
    void publishWeight(float grams);
};

} // namespace core
