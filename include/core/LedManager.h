#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/ILedStrip.h"
#include "hal/IClock.h"

namespace core {

class LedManager final : public events::IEventHandler {
public:
    LedManager(hal::ILedStrip& leds, hal::IClock& clock, events::EventBus& eventBus);

    void begin(uint8_t savedBrightness);
    void loop();

    void onEvent(const events::Event& event) override;

private:
    enum class State : uint8_t {
        Boot,
        Connecting,
        Connected,
        ConnectedFadeout,
        ConfigMode,
        OtaProgress,
        OtaSuccess,
        OtaError,
        ButtonFlash,
        Preview,
        Idle,
    };

    hal::ILedStrip&   _leds;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;

    State    _state         = State::Idle;
    uint32_t _stateStartMs  = 0;
    uint32_t _lastTickMs    = 0;
    uint8_t  _tickPhase     = 0;
    uint8_t  _otaPct        = 0;
    uint8_t  _brightness    = 128;

    void transitionTo(State next);

    void tickBoot();
    void tickConnecting();
    void tickConnected();
    void tickConnectedFadeout();
    void tickConfigMode();
    void tickOtaProgress();
    void tickOtaSuccess();
    void tickOtaError();
    void tickButtonFlash();

    uint32_t elapsed() const;

    State      _prevState    = State::Idle;
    hal::Color _flashColor  = hal::Color::white();
};

} // namespace core
