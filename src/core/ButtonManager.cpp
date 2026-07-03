#include "core/ButtonManager.h"
#include "events/EventType.h"

namespace core {

ButtonManager::ButtonManager(hal::IButton& btn1, hal::IButton& btn2, events::EventBus& eventBus)
    : _btn1(btn1), _btn2(btn2), _eventBus(eventBus) {}

void ButtonManager::begin() {
    _btn1.begin();
    _btn2.begin();

    _btn1.onPress([this]() {
        _eventBus.publish({events::EventType::Button1Pressed});
    });

    _btn2.onPress([this]() {
        _eventBus.publish({events::EventType::Button2Pressed});
    });
}

void ButtonManager::loop() {
    _btn1.tick();
    _btn2.tick();
}

} // namespace core
