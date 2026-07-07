#include "core/BuzzerManager.h"
#include "events/EventType.h"
#include "config.h"

namespace core {

BuzzerManager::BuzzerManager(hal::IBuzzer& buzzer, hal::IClock& clock, events::EventBus& eventBus)
    : _buzzer(buzzer), _clock(clock), _eventBus(eventBus) {}

void BuzzerManager::begin() {
    _buzzer.begin();
    _eventBus.subscribe(events::EventType::Button1Down, this);
    _eventBus.subscribe(events::EventType::Button2Down, this);
}

void BuzzerManager::loop() {
    if (_active && _clock.millis() >= _stopAt) {
        _buzzer.stop();
        _active = false;
    }
}

void BuzzerManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::Button1Down:
        case events::EventType::Button2Down:
            _buzzer.beep(BUZZER_FREQ_HZ, BUZZER_BEEP_MS);
            _stopAt = _clock.millis() + BUZZER_BEEP_MS;
            _active = true;
            break;
        default:
            break;
    }
}

} // namespace core
