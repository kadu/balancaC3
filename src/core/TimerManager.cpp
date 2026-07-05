#include "core/TimerManager.h"
#include "events/EventType.h"

namespace core {

TimerManager::TimerManager(hal::IClock& clock, events::EventBus& eventBus)
    : _clock(clock), _eventBus(eventBus) {}

void TimerManager::begin() {
    _eventBus.subscribe(events::EventType::Button1Pressed,     this);
    _eventBus.subscribe(events::EventType::Button1LongPressed, this);
}

void TimerManager::loop() {
    if (_state != State::Running) return;

    uint32_t totalMs  = elapsedMs();
    uint8_t  seconds  = static_cast<uint8_t>((totalMs / 1000) % 60);

    if (seconds != _lastSeconds) {
        _lastSeconds = seconds;
        publishTick();
    }
}

void TimerManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::Button1Pressed:
            if (_state == State::Stopped || _state == State::Paused) {
                // Start or resume
                _startMs = _clock.millis();
                _state   = State::Running;
                _eventBus.publish({events::EventType::TimerStarted});
                publishTick();
            } else {
                // Pause
                _accumulatedMs += _clock.millis() - _startMs;
                _state = State::Paused;
                _eventBus.publish({events::EventType::TimerPaused});
            }
            break;

        case events::EventType::Button1LongPressed:
            // Reset
            _accumulatedMs = 0;
            _startMs       = 0;
            _lastSeconds   = 0;
            _state         = State::Stopped;
            _payload       = {0, 0};
            _eventBus.publish({events::EventType::TimerReset});
            _eventBus.publish({events::EventType::TimerTick, &_payload});
            break;

        default:
            break;
    }
}

uint32_t TimerManager::elapsedMs() const {
    uint32_t acc = _accumulatedMs;
    if (_state == State::Running) acc += _clock.millis() - _startMs;
    return acc;
}

void TimerManager::publishTick() {
    uint32_t totalSec = elapsedMs() / 1000;
    _payload.minutes  = static_cast<uint16_t>(totalSec / 60);
    _payload.seconds  = static_cast<uint8_t>(totalSec % 60);
    _eventBus.publish({events::EventType::TimerTick, &_payload});
}

} // namespace core
