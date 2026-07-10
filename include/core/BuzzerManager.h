#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IBuzzer.h"
#include "hal/IClock.h"

namespace core {

class BuzzerManager final : public events::IEventHandler {
public:
    BuzzerManager(hal::IBuzzer& buzzer, hal::IClock& clock, events::EventBus& eventBus);

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

private:
    hal::IBuzzer&     _buzzer;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;

    static constexpr uint8_t MAX_BEEPS = 4;

    bool     _active     = false;
    uint32_t _stopAt     = 0;
    uint32_t _nextBeepAt = 0;
    bool     _inGap      = false;

    // Beep sequence queue
    uint32_t _freqs[MAX_BEEPS]     = {};
    uint32_t _durations[MAX_BEEPS] = {};
    uint8_t  _beepHead  = 0;
    uint8_t  _beepCount = 0;
    uint32_t _gapMs     = 80;

    void scheduleBeeps(uint8_t count, uint32_t freqHz, uint32_t durationMs, uint32_t gapMs);
    void scheduleSequence(const uint32_t* freqs, const uint32_t* durations, uint8_t count, uint32_t gapMs);
};

} // namespace core
