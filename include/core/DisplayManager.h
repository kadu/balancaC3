#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IDisplay.h"
#include "hal/IClock.h"

namespace core {

class ScaleManager;

class DisplayManager final : public events::IEventHandler {
public:
    DisplayManager(hal::IDisplay& display, hal::IClock& clock, events::EventBus& eventBus);

    void setScaleManager(ScaleManager* scale) { _scale = scale; }

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
        RecipeMenu,
        RecipeActive,
        RecipeFinished,
    };

    hal::IDisplay&    _display;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;
    ScaleManager*     _scale = nullptr;

    State    _state       = State::Connecting;
    uint32_t _splashStartMs = 0;
    int16_t  _scrollX     = 0;
    int16_t  _scrollTarget = 0;
    uint32_t _lastScrollMs = 0;

    float    _lastWeight  = 0.0f;
    bool     _calibrated  = false;
    char     _ip[16]      = {};
    uint16_t _timerMin     = 0;
    uint8_t  _timerSec     = 0;
    bool     _timerRunning = false;

    // Recipe active mode
    char     _stepType[24]    = {};
    uint32_t _totalElapsed    = 0;
    uint32_t _stepRemaining   = 0;
    uint16_t _stepWater       = 0;
    uint16_t _cumulWater      = 0;
    bool     _stepRunning     = false;
    uint8_t  _stepIndex       = 0;
    uint8_t  _stepTotal       = 0;

    // Recipe finished animation
    uint8_t  _animFrame       = 0;
    uint32_t _animLastMs      = 0;
    static constexpr uint32_t ANIM_HOLD_MS = 4000; // show animation for 4s then go to Scale

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
    void drawRecipeMenu(const void* payload);
    void drawRecipeActive();
};

} // namespace core
