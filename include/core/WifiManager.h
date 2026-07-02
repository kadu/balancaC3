#pragma once

#include "events/EventBus.h"
#include "hal/IWifi.h"
#include "hal/IStorage.h"
#include "hal/ICaptivePortal.h"
#include "hal/IClock.h"

namespace core {

class WifiManager {
public:
    WifiManager(hal::IWifi& wifi, hal::IStorage& storage,
                hal::ICaptivePortal& portal, hal::IClock& clock,
                events::EventBus& eventBus);

    void begin();
    void loop();

private:
    enum class State : uint8_t { Idle, Connecting, Connected, ConfigMode };

    hal::IWifi&          _wifi;
    hal::IStorage&       _storage;
    hal::ICaptivePortal& _portal;
    hal::IClock&         _clock;
    events::EventBus&    _eventBus;

    State    _state          = State::Idle;
    uint32_t _connectStartMs = 0;
    char     _ipBuffer[16]   = {};

    void loadAndConnect();
    void enterConfigMode();
    void transitionTo(State next);

    void tickConnecting();
    void tickConnected();
    void tickConfigMode();
};

} // namespace core
