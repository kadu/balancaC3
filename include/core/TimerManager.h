#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IClock.h"

namespace core {

struct TimerPayload {
    uint16_t minutes;
    uint8_t  seconds;
};

class TimerManager final : public events::IEventHandler {
public:
    TimerManager(hal::IClock& clock, events::EventBus& eventBus);

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

    // Read-only accessors so other components (e.g. the web UI) can show the
    // timer without having to mirror TimerTick events.
    uint32_t elapsedSeconds() const { return elapsedMs() / 1000; }
    bool     isRunning()      const { return _state == State::Running; }

private:
    enum class State : uint8_t { Stopped, Running, Paused };

    hal::IClock&      _clock;
    events::EventBus& _eventBus;

    State    _state        = State::Stopped;
    uint32_t _startMs      = 0;   // millis when last started/resumed
    uint32_t _accumulatedMs = 0;  // ms accumulated before last pause
    uint32_t _lastTickMs   = 0;
    uint8_t  _lastSeconds  = 0;

    TimerPayload _payload{};

    uint32_t elapsedMs() const;
    void     publishTick();
};

} // namespace core
