#include "core/ButtonManager.h"
#include "events/EventType.h"
#include "config.h"

namespace core {

ButtonManager::ButtonManager(hal::IButton& btn1, hal::IButton& btn2, events::EventBus& eventBus)
    : _btn1(btn1), _btn2(btn2), _eventBus(eventBus) {}

void ButtonManager::begin() {
    _btn1.begin();
    _btn2.begin();

    _btn1.setTimings(BUTTON_DEBOUNCE_MS, BUTTON_CLICK_MS, BUTTON_LONG_PRESS_MS);
    _btn2.setTimings(BUTTON_DEBOUNCE_MS, BUTTON_CLICK_MS, BUTTON_LONG_PRESS_MS);

    _btn1.ignoreFirstMs(BUTTON_STARTUP_MS);
    _btn2.ignoreFirstMs(BUTTON_STARTUP_MS);

    _btn1.onDown([this]() {
        _eventBus.publish({events::EventType::Button1Down});
    });

    _btn1.onPress([this]() {
        _eventBus.publish({events::EventType::Button1Pressed});
    });

    _btn1.onLongPress([this]() {
        _eventBus.publish({events::EventType::Button1LongPressed});
    });

    _btn2.onDown([this]() {
        _eventBus.publish({events::EventType::Button2Down});
    });

    _btn2.onPress([this]() {
        _eventBus.publish({events::EventType::Button2Pressed});
    });

    _btn2.onLongPress([this]() {
        _eventBus.publish({events::EventType::Button2LongPressed});
    });

    // So registra o clique duplo se ele for usado: o OneButton passa a segurar o
    // clique simples por BUTTON_CLICK_MS assim que existe um handler de duplo,
    // e essa espera pesa na tara, que e o gesto mais usado.
    if (BUTTON_DOUBLE_CLICK) {
        _btn1.onDoubleClick([this]() {
            _eventBus.publish({events::EventType::Button1DoubleClick});
        });

        _btn2.onDoubleClick([this]() {
            _eventBus.publish({events::EventType::Button2DoubleClick});
        });
    }
}

void ButtonManager::loop() {
    _btn1.tick();
    _btn2.tick();
}

} // namespace core
