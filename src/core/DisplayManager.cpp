#include "core/DisplayManager.h"
#include "core/ScaleManager.h"
#include "core/TimerManager.h"
#include "events/EventType.h"
#include "logo.h"
#include <cstdio>
#include <cstring>

namespace core {

DisplayManager::DisplayManager(hal::IDisplay& display, hal::IClock& clock, events::EventBus& eventBus)
    : _display(display), _clock(clock), _eventBus(eventBus) {}

void DisplayManager::begin() {
    _display.begin();

    _eventBus.subscribe(events::EventType::WifiConnecting,     this);
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);
    _eventBus.subscribe(events::EventType::WeightUpdated,      this);
    _eventBus.subscribe(events::EventType::ScaleTared,         this);
    _eventBus.subscribe(events::EventType::ScaleCalibrated,    this);
    _eventBus.subscribe(events::EventType::TimerTick,          this);
    _eventBus.subscribe(events::EventType::TimerReset,         this);

    transitionTo(State::SplashLogo);
}

void DisplayManager::loop() {
    uint32_t now = _clock.millis();

    if (_state == State::SplashLogo) {
        if (now - _splashStartMs >= SPLASH_LOGO_MS) {
            // If IP already received during logo, go straight to connected splash
            if (_ip[0] != '\0') transitionTo(State::SplashConnected);
            else                transitionTo(State::Connecting);
        }
        return;
    }

    if (_state == State::SplashConnected) {
        // Advance scroll
        if (now - _lastScrollMs >= SCROLL_INTERVAL_MS) {
            _lastScrollMs = now;
            if (_scrollX > _scrollTarget) {
                _scrollX -= 2;
                if (_scrollX < _scrollTarget) _scrollX = _scrollTarget;
            }
            drawSplashConnected();
        }
        // Transition after timeout
        if (now - _splashStartMs >= SPLASH_CONNECTED_MS) transitionTo(State::Scale);
        return;
    }

    if (_state == State::SplashAp) {
        if (now - _splashStartMs >= SPLASH_AP_MS) transitionTo(State::Scale);
        return;
    }
}

void DisplayManager::onEvent(const events::Event& event) {
    // Hold logo splash — buffer WiFi events until logo finishes
    if (_state == State::SplashLogo) {
        if (event.type == events::EventType::WifiConnected && event.payload)
            strncpy(_ip, static_cast<const char*>(event.payload), sizeof(_ip) - 1);
        return;
    }

    switch (event.type) {
        case events::EventType::WifiConnecting:
            transitionTo(State::Connecting);
            break;
        case events::EventType::WifiConnected:
            if (event.payload) strncpy(_ip, static_cast<const char*>(event.payload), sizeof(_ip) - 1);
            transitionTo(State::SplashConnected);
            break;
        case events::EventType::WifiDisconnected:
            transitionTo(State::Connecting);
            break;
        case events::EventType::WifiConfigRequired:
            transitionTo(State::SplashAp);
            break;
        case events::EventType::WeightUpdated: {
            auto* p = static_cast<const WeightPayload*>(event.payload);
            if (p) { _lastWeight = p->grams; _calibrated = p->calibrated; }
            if (_state == State::Scale) drawScale();
            break;
        }
        case events::EventType::ScaleTared:
            _lastWeight = 0.0f;
            if (_state == State::Scale) drawScale();
            break;
        case events::EventType::ScaleCalibrated:
            _calibrated = true;
            if (_state == State::Scale) drawScale();
            break;
        case events::EventType::TimerTick:
        case events::EventType::TimerReset: {
            auto* p = static_cast<const TimerPayload*>(event.payload);
            if (p) { _timerMin = p->minutes; _timerSec = p->seconds; }
            if (_state == State::Scale) drawScale();
            break;
        }
        default:
            break;
    }
}

void DisplayManager::transitionTo(State next) {
    _state = next;

    switch (_state) {
        case State::SplashLogo:
            _splashStartMs = _clock.millis();
            drawSplashLogo();
            break;
        case State::Connecting:
            drawConnecting();
            break;
        case State::SplashConnected: {
            _splashStartMs = _clock.millis();
            _lastScrollMs  = _splashStartMs;
            // Start text off-screen to the right
            _display.setFontLarge();
            uint8_t w = _display.stringWidth(_ip);
            _scrollX      = static_cast<int16_t>(_display.displayWidth());
            _scrollTarget = static_cast<int16_t>((_display.displayWidth() - w) / 2);
            drawSplashConnected();
            break;
        }
        case State::SplashAp:
            _splashStartMs = _clock.millis();
            drawSplashAp();
            break;
        case State::Scale:
            drawScale();
            break;
    }
}

// ── Scale screen ─────────────────────────────────────────────────────────────
//
//  line 28  │     2:05          (XLarge ~28px)
//  line 40  ├──────────────────
//  line 62  │   250.3 g        (XLarge ~28px)
//
void DisplayManager::drawScale() {
    _display.clear();

    // Timer
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%u:%02u", _timerMin, _timerSec);
    _display.setFontLarge();
    _display.drawStringCenter(20, timeBuf);

    // Weight
    if (!_calibrated) {
        _display.setFontSmall();
        _display.drawStringCenter(44, "Sem calibracao");
        _display.drawStringCenter(56, "Acesse /config");
    } else {
        char wBuf[12];
        if (_lastWeight >= 1000.0f || _lastWeight <= -1000.0f)
            snprintf(wBuf, sizeof(wBuf), "%.2fkg", _lastWeight / 1000.0f);
        else
            snprintf(wBuf, sizeof(wBuf), "%.1fg", _lastWeight);
        _display.setFontXLarge();
        _display.drawStringCenter(56, wBuf);
    }

    _display.show();
}

// ── Splash: IP scroll ────────────────────────────────────────────────────────
void DisplayManager::drawSplashConnected() {
    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(12, "Conectado!");
    _display.drawHLine(0, 16, 128);
    _display.setFontLarge();
    _display.drawStringAt(_scrollX, 44, _ip);
    _display.show();
}

// ── Splash: AP mode ──────────────────────────────────────────────────────────
void DisplayManager::drawSplashAp() {
    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(14, "Modo configuracao");
    _display.drawHLine(0, 18, 128);
    _display.drawStringCenter(34, "Conecte-se na rede:");
    _display.setFontSmall();
    _display.drawStringCenter(48, "BalancaC3-Config");
    _display.drawHLine(0, 54, 128);
    _display.drawStringCenter(64, "192.168.4.1");
    _display.show();
}

// ── Logo splash ──────────────────────────────────────────────────────────────
void DisplayManager::drawSplashLogo() {
    _display.clear();
    _display.drawBitmap(0, 0, LOGO_WIDTH, LOGO_HEIGHT, logo_bitmap);
    _display.show();
}

// ── Connecting ───────────────────────────────────────────────────────────────
void DisplayManager::drawConnecting() {
    _display.clear();
    _display.setFontMedium();
    _display.drawStringCenter(30, "WiFi");
    _display.setFontSmall();
    _display.drawStringCenter(48, "Conectando...");
    _display.show();
}

} // namespace core
