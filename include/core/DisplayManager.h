#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IDisplay.h"
#include "hal/IClock.h"

namespace core {

class DisplayManager final : public events::IEventHandler {
public:
    DisplayManager(hal::IDisplay& display, hal::IClock& clock, events::EventBus& eventBus);

    void begin();
    void loop();
    void onEvent(const events::Event& event) override;

private:
    enum class State : uint8_t {
        SplashLogo,
        Connecting,
        SplashConnected,
        SplashAp,
        Scale,
    };

    hal::IDisplay&    _display;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;

    State    _state       = State::Connecting;
    uint32_t _splashStartMs = 0;
    int16_t  _scrollX     = 0;
    int16_t  _scrollTarget = 0;
    uint32_t _lastScrollMs = 0;

    float    _lastWeight  = 0.0f;
    bool     _calibrated  = false;
    char     _ip[16]      = {};
    uint16_t _timerMin    = 0;
    uint8_t  _timerSec    = 0;

    static constexpr uint32_t SPLASH_LOGO_MS      = 3000;
    static constexpr uint32_t SPLASH_CONNECTED_MS = 3000;
    static constexpr uint32_t SPLASH_AP_MS        = 2500;
    static constexpr uint32_t SCROLL_INTERVAL_MS  = 20;

    void transitionTo(State next);
    void drawScale();
    void drawConnecting();
    void drawSplashLogo();
    void drawSplashConnected();
    void drawSplashAp();
};

} // namespace core
