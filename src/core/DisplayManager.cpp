#include "core/DisplayManager.h"
#include "events/EventType.h"

namespace core {

DisplayManager::DisplayManager(hal::IDisplay& display, events::EventBus& eventBus)
    : _display(display), _eventBus(eventBus) {}

void DisplayManager::begin() {
    _display.begin();

    _eventBus.subscribe(events::EventType::WifiConnecting,     this);
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);

    showConnecting();
}

void DisplayManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::WifiConnecting:
            showConnecting();
            break;
        case events::EventType::WifiConnected:
            showConnected(static_cast<const char*>(event.payload));
            break;
        case events::EventType::WifiDisconnected:
            showConnecting();
            break;
        case events::EventType::WifiConfigRequired:
            showApMode();
            break;
        default:
            break;
    }
}

void DisplayManager::showConnected(const char* ip) {
    _display.clear();
    _display.setFontMedium();
    _display.drawStringCenter(20, "WiFi");
    _display.setFontSmall();
    _display.drawStringCenter(34, "Conectado");
    _display.drawHLine(0, 40, 128);
    _display.setFontMedium();
    _display.drawStringCenter(58, ip ? ip : "");
    _display.show();
}

void DisplayManager::showApMode() {
    _display.clear();
    _display.setFontSmall();
    _display.drawStringCenter(12, "Modo configuracao");
    _display.drawHLine(0, 16, 128);
    _display.setFontSmall();
    _display.drawStringCenter(30, "Conecte no WiFi:");
    _display.setFontMedium();
    _display.drawStringCenter(46, "BalancaC3-Config");
    _display.drawHLine(0, 50, 128);
    _display.setFontSmall();
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
