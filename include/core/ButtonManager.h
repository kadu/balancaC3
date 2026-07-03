#pragma once

#include "events/EventBus.h"
#include "hal/IButton.h"

namespace core {

class ButtonManager {
public:
    ButtonManager(hal::IButton& btn1, hal::IButton& btn2, events::EventBus& eventBus);

    void begin();
    void loop();

private:
    hal::IButton&     _btn1;
    hal::IButton&     _btn2;
    events::EventBus& _eventBus;
};

} // namespace core
