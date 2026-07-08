#include "core/DisplayManager.h"
#include "core/ScaleManager.h"
#include "core/TimerManager.h"
#include "core/RecipeManager.h"
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
    _eventBus.subscribe(events::EventType::TimerStarted,       this);
    _eventBus.subscribe(events::EventType::TimerPaused,        this);
    _eventBus.subscribe(events::EventType::RecipeMenuOpen,     this);
    _eventBus.subscribe(events::EventType::RecipeCancelled,    this);
    _eventBus.subscribe(events::EventType::RecipeSelected,     this);

    transitionTo(State::SplashLogo);
}

void DisplayManager::loop() {
    uint32_t now = _clock.millis();

    if (_state == State::SplashLogo) {
        if (now - _splashStartMs >= SPLASH_LOGO_MS) {
            // Sync calibration state from ScaleManager before first draw
            if (_scale) _calibrated = _scale->isCalibrated();
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
        case events::EventType::TimerStarted:
            _timerRunning = true;
            if (_state == State::Scale) drawScale();
            break;
        case events::EventType::TimerPaused:
            _timerRunning = false;
            if (_state == State::Scale) drawScale();
            break;
        case events::EventType::RecipeMenuOpen:
            _state = State::RecipeMenu;
            drawRecipeMenu(event.payload);
            break;
        case events::EventType::RecipeCancelled:
        case events::EventType::RecipeSelected:
            if (_state == State::RecipeMenu) {
                transitionTo(State::Scale);
            }
            break;
        case events::EventType::TimerTick: {
            auto* p = static_cast<const TimerPayload*>(event.payload);
            if (p) { _timerMin = p->minutes; _timerSec = p->seconds; }
            if (_state == State::Scale) drawScale();
            break;
        }
        case events::EventType::TimerReset: {
            auto* p = static_cast<const TimerPayload*>(event.payload);
            if (p) { _timerMin = p->minutes; _timerSec = p->seconds; }
            _timerRunning = false;
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

    // Timer — icon + text centered together
    char timeBuf[8];
    snprintf(timeBuf, sizeof(timeBuf), "%u:%02u", _timerMin, _timerSec);
    _display.setFontLarge();
    uint8_t tw  = _display.stringWidth(timeBuf);
    // icon is 7px wide + 4px gap
    uint8_t iconW = 7;
    uint8_t gap   = 4;
    uint8_t totalW = iconW + gap + tw;
    uint8_t startX = (128 - totalW) / 2;
    uint8_t iconTop = 20 - 12; // baseline 20, font ~14px tall → top at ~6

    if (_timerRunning) {
        // ▶ play triangle: points at (x,top), (x, top+12), (x+7, top+6)
        _display.drawTriangle(startX, iconTop, startX, iconTop + 12, startX + iconW, iconTop + 6);
    } else {
        // ⏸ pause: two vertical bars 3px wide with 1px gap
        _display.drawBox(startX,         iconTop, 3, 12);
        _display.drawBox(startX + 4,     iconTop, 3, 12);
    }
    _display.drawStringAt(startX + iconW + gap, 20, timeBuf);

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

// ── Recipe menu ──────────────────────────────────────────────────────────────
//
//  line  8  │ RECEITAS              (small, center)
//  line 14  ├────────────────────── (hline)
//  lines    │   Receita 1           each item ~12px apart
//           │ > Receita 2  ←selected (arrow + inverted or bold)
//           │   Receita 3
//           │   Sem Receita
//  line 62  │ [1] nav  [2L] select  (hint)
//
void DisplayManager::drawRecipeMenu(const void* payload) {
    if (!payload) return;
    auto* p = static_cast<const RecipeMenuPayload*>(payload);

    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(8, "RECEITAS");
    _display.drawHLine(0, 11, 128);

    // 3 visible items with scroll window following selection
    // Layout: y = 22, 34, 46  →  hline 52  →  hint 62
    const uint8_t VISIBLE = 3;
    uint8_t startIdx = 0;
    if (p->selectedIndex >= VISIBLE) startIdx = p->selectedIndex - VISIBLE + 1;

    for (uint8_t i = 0; i < VISIBLE && (startIdx + i) < p->itemCount; ++i) {
        uint8_t idx = startIdx + i;
        uint8_t y   = 22 + i * 12;
        bool    sel = (idx == p->selectedIndex);
        if (sel) {
            _display.drawString(2,  y, ">");
            _display.drawString(12, y, p->items[idx].name);
        } else {
            _display.drawString(12, y, p->items[idx].name);
        }
    }

    // Scroll indicators
    if (startIdx > 0)
        _display.drawString(122, 22, "^");
    if (startIdx + VISIBLE < p->itemCount)
        _display.drawString(122, 46, "v");

    _display.drawHLine(0, 52, 128);
    _display.drawString(2,  62, "[1]nav");
    _display.drawStringAt(68, 62, "[2L]ok");
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
