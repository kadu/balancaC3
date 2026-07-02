#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/ISerial.h"
#include "hal/IClock.h"

namespace core {

static constexpr uint8_t  HELLO_WORLD_COUNT    = 5;
static constexpr uint32_t HELLO_WORLD_INTERVAL = 1000;

class Application final : public events::IEventHandler {
public:
    Application(hal::ISerial& serial, hal::IClock& clock, events::EventBus& eventBus);

    void setup();
    void loop();

    void onEvent(const events::Event& event) override;

private:
    hal::ISerial&      _serial;
    hal::IClock&       _clock;
    events::EventBus&  _eventBus;

    uint8_t  _helloCount   = 0;
    uint32_t _lastPrintMs  = 0;
    bool     _printing     = false;
};

} // namespace core
