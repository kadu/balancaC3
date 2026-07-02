#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/ISerial.h"

namespace core {

class Application final : public events::IEventHandler {
public:
    explicit Application(hal::ISerial& serial, events::EventBus& eventBus);

    void setup();
    void loop();

    void onEvent(const events::Event& event) override;

private:
    hal::ISerial&      _serial;
    events::EventBus&  _eventBus;
};

} // namespace core
