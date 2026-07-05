#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IDisplay.h"

namespace core {

class DisplayManager final : public events::IEventHandler {
public:
    DisplayManager(hal::IDisplay& display, events::EventBus& eventBus);

    void begin();
    void onEvent(const events::Event& event) override;

private:
    hal::IDisplay&    _display;
    events::EventBus& _eventBus;

    bool  _connected   = false;
    float _lastWeight  = 0.0f;
    bool  _calibrated  = false;
    char  _ip[16]      = {};

    void showConnected();
    void showWeight(float grams, bool calibrated);
    void showApMode();
    void showConnecting();
};

} // namespace core
