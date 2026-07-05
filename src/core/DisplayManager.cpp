#include "core/DisplayManager.h"
#include "core/ScaleManager.h"
#include "events/EventType.h"
#include <cstdio>
#include <cstring>

namespace core {

DisplayManager::DisplayManager(hal::IDisplay& display, events::EventBus& eventBus)
    : _display(display), _eventBus(eventBus) {}

void DisplayManager::begin() {
    _display.begin();

    _eventBus.subscribe(events::EventType::WifiConnecting,     this);
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);
    _eventBus.subscribe(events::EventType::WeightUpdated,      this);
    _eventBus.subscribe(events::EventType::ScaleTared,         this);
    _eventBus.subscribe(events::EventType::ScaleCalibrated,    this);

    showConnecting();
}

void DisplayManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::WifiConnecting:
            _connected = false;
            showConnecting();
            break;
        case events::EventType::WifiConnected:
            _connected = true;
            if (event.payload) strncpy(_ip, static_cast<const char*>(event.payload), sizeof(_ip) - 1);
            showConnected();
            break;
        case events::EventType::WifiDisconnected:
            _connected = false;
            showConnecting();
            break;
        case events::EventType::WifiConfigRequired:
            _connected = false;
            showApMode();
            break;
        case events::EventType::WeightUpdated: {
            auto* p = static_cast<const WeightPayload*>(event.payload);
            if (p) { _lastWeight = p->grams; _calibrated = p->calibrated; }
            showWeight(_lastWeight, _calibrated);
            break;
        }
        case events::EventType::ScaleTared:
            showWeight(0.0f, _calibrated);
            break;
        case events::EventType::ScaleCalibrated:
            _calibrated = true;
            showWeight(_lastWeight, true);
            break;
        default:
            break;
    }
}

void DisplayManager::showConnected() {
    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(10, _ip);
    _display.drawHLine(0, 14, 128);
    showWeight(_lastWeight, _calibrated);
}

void DisplayManager::showWeight(float grams, bool calibrated) {
    if (!_connected) return;

    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(10, _ip);
    _display.drawHLine(0, 14, 128);

    if (!calibrated) {
        _display.setFontSmall();
        _display.drawStringCenter(35, "Sem calibracao");
        _display.drawStringCenter(50, "Acesse /config");
    } else {
        // Format: "1234.5 g" or "1.234 kg" above 1000g
        char buf[16];
        if (grams >= 1000.0f || grams <= -1000.0f) {
            snprintf(buf, sizeof(buf), "%.3f kg", grams / 1000.0f);
        } else {
            snprintf(buf, sizeof(buf), "%.1f g", grams);
        }
        _display.setFontLarge();
        _display.drawStringCenter(48, buf);
        _display.setFontSmall();
        _display.drawStringCenter(62, "balanca de cafe");
    }
    _display.show();
}

void DisplayManager::showApMode() {
    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(12, "Modo configuracao");
    _display.drawHLine(0, 16, 128);
    _display.drawStringCenter(30, "Conecte no WiFi:");
    _display.setFontSmall();
    _display.drawStringCenter(44, "BalancaC3-Config");
    _display.drawHLine(0, 50, 128);
    _display.drawStringCenter(62, "192.168.4.1");
    _display.show();
}

void DisplayManager::showConnecting() {
    _display.clear();
    _display.setFontMedium();
    _display.drawStringCenter(30, "WiFi");
    _display.setFontSmall();
    _display.drawStringCenter(48, "Conectando...");
    _display.show();
}

} // namespace core
