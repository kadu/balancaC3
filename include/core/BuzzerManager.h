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

    bool     _active   = false;
    uint32_t _stopAt   = 0;
};

} // namespace core
