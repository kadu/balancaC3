#include "core/Application.h"
#include "events/EventType.h"

namespace core {

Application::Application(hal::ISerial& serial, hal::IClock& clock, events::EventBus& eventBus)
    : _serial(serial), _clock(clock), _eventBus(eventBus) {}

void Application::setup() {
    _serial.begin(115200);

    _eventBus.subscribe(events::EventType::SystemStarted, this);
    _eventBus.subscribe(events::EventType::SystemReady,   this);

    _eventBus.publish({events::EventType::SystemStarted});
}

void Application::loop() {
    _eventBus.dispatch();

    if (!_printing) return;

    uint32_t now = _clock.millis();
    if (now - _lastPrintMs < HELLO_WORLD_INTERVAL) return;

    _lastPrintMs = now;
    _serial.println("Hello World!");
    _helloCount++;

    if (_helloCount >= HELLO_WORLD_COUNT) {
        _printing = false;
        _eventBus.publish({events::EventType::SystemReady});
    }
}

void Application::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::SystemStarted:
            _helloCount  = 0;
            _lastPrintMs = _clock.millis();
            _printing    = true;
            break;

        case events::EventType::SystemReady:
            _serial.println("[System] Ready.");
            break;

        default:
            break;
    }
}

} // namespace core
