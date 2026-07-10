#include "core/LedManager.h"
#include "core/RecipeManager.h"
#include "events/EventType.h"
#include <cmath>

// Breathing: full sine cycle in BREATH_PERIOD_MS milliseconds
static constexpr uint32_t BREATH_PERIOD_MS  = 3000;
static constexpr uint32_t BLINK_PERIOD_MS   = 500;
static constexpr uint32_t CONNECTED_HOLD_MS = 3000;
static constexpr uint32_t FADEOUT_MS        = 1500;
static constexpr uint32_t OTA_ERROR_MS      = 1200; // 3 flashes × 400ms
static constexpr uint32_t OTA_SUCCESS_MS    = 1000;

namespace core {

LedManager::LedManager(hal::ILedStrip& leds, hal::IClock& clock, events::EventBus& eventBus)
    : _leds(leds), _clock(clock), _eventBus(eventBus) {}

void LedManager::begin(uint8_t savedBrightness) {
    _brightness = savedBrightness;
    _leds.begin();
    _leds.setBrightness(_brightness);

    _eventBus.subscribe(events::EventType::SystemStarted,       this);
    _eventBus.subscribe(events::EventType::WifiConnecting,      this);
    _eventBus.subscribe(events::EventType::WifiConnected,       this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,    this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired,  this);
    _eventBus.subscribe(events::EventType::OtaStarted,          this);
    _eventBus.subscribe(events::EventType::OtaProgress,         this);
    _eventBus.subscribe(events::EventType::OtaSuccess,          this);
    _eventBus.subscribe(events::EventType::OtaError,            this);
    _eventBus.subscribe(events::EventType::LedBrightnessChanged, this);
    _eventBus.subscribe(events::EventType::LedPreviewChanged,    this);
    _eventBus.subscribe(events::EventType::LedPreviewStopped,    this);
    _eventBus.subscribe(events::EventType::Button1Down,           this);
    _eventBus.subscribe(events::EventType::Button2Down,           this);
    _eventBus.subscribe(events::EventType::Button1Pressed,        this);
    _eventBus.subscribe(events::EventType::Button2Pressed,        this);
    _eventBus.subscribe(events::EventType::RecipeStepStarted,     this);
    _eventBus.subscribe(events::EventType::RecipeStepTick,        this);
    _eventBus.subscribe(events::EventType::RecipeStepCompleted,   this);
    _eventBus.subscribe(events::EventType::RecipeFinished,        this);
    _eventBus.subscribe(events::EventType::RecipeCancelled,       this);
}

void LedManager::loop() {
    switch (_state) {
        case State::Boot:             tickBoot();             break;
        case State::Connecting:       tickConnecting();       break;
        case State::Connected:        tickConnected();        break;
        case State::ConnectedFadeout: tickConnectedFadeout(); break;
        case State::ConfigMode:       tickConfigMode();       break;
        case State::OtaProgress:      tickOtaProgress();      break;
        case State::OtaSuccess:       tickOtaSuccess();       break;
        case State::OtaError:         tickOtaError();         break;
        case State::ButtonFlash:      tickButtonFlash();      break;
        case State::Preview:                                  break;
        case State::RecipeProgress:                           break;
        case State::Idle:                                     break;
    }
}

void LedManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::SystemStarted:
            transitionTo(State::Boot);
            break;
        case events::EventType::WifiConnecting:
            transitionTo(State::Connecting);
            break;
        case events::EventType::WifiConnected:
            transitionTo(State::Connected);
            break;
        case events::EventType::WifiDisconnected:
            transitionTo(State::Connecting);
            break;
        case events::EventType::WifiConfigRequired:
            transitionTo(State::ConfigMode);
            break;
        case events::EventType::OtaStarted:
            _otaPct = 0;
            transitionTo(State::OtaProgress);
            break;
        case events::EventType::OtaProgress:
            if (event.payload) _otaPct = *static_cast<const uint8_t*>(event.payload);
            break;
        case events::EventType::OtaSuccess:
            transitionTo(State::OtaSuccess);
            break;
        case events::EventType::OtaError:
            transitionTo(State::OtaError);
            break;
        case events::EventType::Button1Down:
            if (_state != State::OtaProgress && _state != State::OtaError) {
                _prevState  = _state;
                _flashColor = hal::Color::green();
                transitionTo(State::ButtonFlash);
            }
            break;
        case events::EventType::Button2Down:
            if (_state != State::OtaProgress && _state != State::OtaError) {
                _prevState  = _state;
                _flashColor = hal::Color::red();
                transitionTo(State::ButtonFlash);
            }
            break;
        case events::EventType::Button1Pressed:
        case events::EventType::Button2Pressed:
            // Action already handled by other managers; extinguish flash
            if (_state == State::ButtonFlash) transitionTo(_prevState);
            break;
        case events::EventType::RecipeStepStarted: {
            auto* p = static_cast<const RecipeStepPayload*>(event.payload);
            if (p && p->stepDurationSecs > 0) {
                _recipeTotal     = p->stepDurationSecs;
                _recipeRemaining = p->remainingSecs;
                _recipeRunning   = p->running;
                transitionTo(State::RecipeProgress);
            } else {
                transitionTo(State::Idle);
            }
            break;
        }
        case events::EventType::RecipeStepTick: {
            auto* p = static_cast<const RecipeStepPayload*>(event.payload);
            if (p && _state == State::RecipeProgress) {
                _recipeRemaining = p->remainingSecs;
                _recipeRunning   = p->running;
                _leds.setBrightness(_brightness);
                if (!_recipeRunning) {
                    _leds.setAll(hal::Color::darkBlue());
                } else {
                    drawRecipeProgress(_recipeRemaining, _recipeTotal);
                }
                _leds.show();
            }
            break;
        }
        case events::EventType::RecipeStepCompleted:
        case events::EventType::RecipeFinished:
        case events::EventType::RecipeCancelled:
            if (_state == State::RecipeProgress) transitionTo(State::Idle);
            break;

        case events::EventType::LedBrightnessChanged:
            if (event.payload) {
                _brightness = *static_cast<const uint8_t*>(event.payload);
                _leds.setBrightness(_brightness);
                _leds.show();
            }
            break;

        case events::EventType::LedPreviewChanged:
            if (event.payload) {
                uint8_t b = *static_cast<const uint8_t*>(event.payload);
                _leds.setBrightness(b);
                _leds.setAll(hal::Color::green());
                _leds.show();
                if (_state != State::Preview) {
                    _prevState = _state;
                    _state     = State::Preview;
                }
            }
            break;

        case events::EventType::LedPreviewStopped:
            if (_state == State::Preview) {
                _leds.setBrightness(_brightness);
                transitionTo(_prevState);
            }
            break;
        default:
            break;
    }
}

void LedManager::transitionTo(State next) {
    _state        = next;
    _stateStartMs = _clock.millis();
    _lastTickMs   = _stateStartMs;
    _tickPhase    = 0;

    // Immediate frame on entry
    switch (_state) {
        case State::RecipeProgress:
            _leds.setBrightness(_brightness);
            if (!_recipeRunning) {
                _leds.setAll(hal::Color::darkBlue());
            } else {
                drawRecipeProgress(_recipeRemaining, _recipeTotal);
            }
            _leds.show();
            break;
        case State::ButtonFlash:
            _leds.setBrightness(_brightness);
            _leds.setAll(_flashColor);
            _leds.show();
            break;
        case State::Boot:
            _leds.setAll(hal::Color::white());
            _leds.show();
            break;
        case State::Idle:
            _leds.clear();
            _leds.show();
            break;
        default:
            break;
    }
}

uint32_t LedManager::elapsed() const {
    return _clock.millis() - _stateStartMs;
}

// ── Boot ────────────────────────────────────────────────────────────────────
void LedManager::tickBoot() {
    // White for 800ms then transition to Idle (WifiConnecting will follow)
    if (elapsed() > 800) transitionTo(State::Idle);
}

// ── Connecting — blink green 500ms ──────────────────────────────────────────
void LedManager::tickConnecting() {
    uint32_t now = _clock.millis();
    if (now - _lastTickMs < BLINK_PERIOD_MS) return;
    _lastTickMs = now;
    _tickPhase ^= 1;
    if (_tickPhase) _leds.setAll(hal::Color::green());
    else            _leds.clear();
    _leds.show();
}

// ── Connected — solid green for 3s ──────────────────────────────────────────
void LedManager::tickConnected() {
    if (_tickPhase == 0) {
        _leds.setAll(hal::Color::green());
        _leds.show();
        _tickPhase = 1;
    }
    if (elapsed() >= CONNECTED_HOLD_MS) transitionTo(State::ConnectedFadeout);
}

// ── Connected fadeout — 1.5s linear dim ─────────────────────────────────────
void LedManager::tickConnectedFadeout() {
    uint32_t now = _clock.millis();
    if (now - _lastTickMs < 30) return; // ~33fps
    _lastTickMs = now;

    uint32_t t = elapsed();
    if (t >= FADEOUT_MS) {
        _leds.setBrightness(_brightness);
        transitionTo(State::Idle);
        return;
    }
    uint8_t dim = static_cast<uint8_t>(_brightness * (FADEOUT_MS - t) / FADEOUT_MS);
    _leds.setBrightness(dim);
    _leds.setAll(hal::Color::green());
    _leds.show();
}

// ── Config mode — breathing yellow ──────────────────────────────────────────
void LedManager::tickConfigMode() {
    uint32_t now = _clock.millis();
    if (now - _lastTickMs < 20) return; // 50fps
    _lastTickMs = now;

    float phase = (float)(now % BREATH_PERIOD_MS) / BREATH_PERIOD_MS;
    float sine  = (sinf(phase * 2.0f * 3.14159f) + 1.0f) * 0.5f; // 0..1
    uint8_t val = static_cast<uint8_t>(40 + sine * 215); // 40..255 range
    _leds.setBrightness(val);
    _leds.setAll(hal::Color::yellow());
    _leds.show();
}

// ── OTA progress — blue bar ──────────────────────────────────────────────────
void LedManager::tickOtaProgress() {
    uint32_t now = _clock.millis();
    if (now - _lastTickMs < 50) return;
    _lastTickMs = now;

    _leds.setBrightness(_brightness);
    _leds.clear();
    uint8_t lit = static_cast<uint8_t>((uint16_t)_otaPct * _leds.count() / 100);
    for (uint8_t i = 0; i < lit; ++i) _leds.setOne(i, hal::Color::blue());
    _leds.show();
}

// ── OTA success — solid blue then fadeout ───────────────────────────────────
void LedManager::tickOtaSuccess() {
    if (_tickPhase == 0) {
        _leds.setBrightness(_brightness);
        _leds.setAll(hal::Color::blue());
        _leds.show();
        _tickPhase = 1;
    }
    if (elapsed() >= OTA_SUCCESS_MS) transitionTo(State::Idle);
}

// ── OTA error — 3× red flash ─────────────────────────────────────────────────
void LedManager::tickOtaError() {
    uint32_t now = _clock.millis();
    if (now - _lastTickMs < 200) return;
    _lastTickMs = now;

    _tickPhase ^= 1;
    _leds.setBrightness(_brightness);
    if (_tickPhase) _leds.setAll(hal::Color::red());
    else            _leds.clear();
    _leds.show();

    if (elapsed() >= OTA_ERROR_MS) transitionTo(State::Idle);
}

// ── Recipe progress — light blue bar draining left to right ─────────────────
void LedManager::drawRecipeProgress(uint32_t remaining, uint32_t total) {
    if (total == 0) return;
    _leds.setBrightness(_brightness);
    _leds.clear();
    uint8_t count = _leds.count();
    // Number of lit LEDs proportional to remaining time
    uint8_t lit = static_cast<uint8_t>((uint32_t)remaining * count / total);
    for (uint8_t i = 0; i < lit; ++i) _leds.setOne(i, hal::Color::lightBlue());
    _leds.show();
}

// ── Button flash — on while pressed, timeout of 2s as safety fallback ───────
void LedManager::tickButtonFlash() {
    if (elapsed() >= 2000) transitionTo(_prevState);
}

} // namespace core
