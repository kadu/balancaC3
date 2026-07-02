#include "core/WifiManager.h"
#include "config.h"
#include <cstring>

namespace core {

WifiManager::WifiManager(hal::IWifi& wifi, hal::IStorage& storage,
                         hal::ICaptivePortal& portal, hal::IClock& clock,
                         events::EventBus& eventBus)
    : _wifi(wifi), _storage(storage), _portal(portal),
      _clock(clock), _eventBus(eventBus) {}

void WifiManager::begin() {
    loadAndConnect();
}

void WifiManager::loop() {
    switch (_state) {
        case State::Connecting:  tickConnecting();  break;
        case State::Connected:   tickConnected();   break;
        case State::ConfigMode:  tickConfigMode();  break;
        default: break;
    }
}

void WifiManager::loadAndConnect() {
    char ssid[33] = {};
    char pass[65] = {};

    bool hasSSID = _storage.getString(STORAGE_KEY_WIFI_SSID, ssid, sizeof(ssid));

    if (hasSSID && ssid[0] != '\0') {
        _storage.getString(STORAGE_KEY_WIFI_PASS, pass, sizeof(pass));
        _wifi.connect(ssid, pass);
        transitionTo(State::Connecting);
    } else {
        enterConfigMode();
    }
}

void WifiManager::enterConfigMode() {
    _wifi.startAP(WIFI_AP_SSID);
    _portal.begin();
    transitionTo(State::ConfigMode);
}

void WifiManager::transitionTo(State next) {
    if (next == _state) return;
    _state = next;

    switch (_state) {
        case State::Connecting:
            _connectStartMs = _clock.millis();
            _eventBus.publish({events::EventType::WifiConnecting});
            break;
        case State::Connected:
            _wifi.localIP(_ipBuffer, sizeof(_ipBuffer));
            _eventBus.publish({events::EventType::WifiConnected, _ipBuffer});
            break;
        case State::ConfigMode:
            _eventBus.publish({events::EventType::WifiConfigRequired});
            break;
        default:
            break;
    }
}

void WifiManager::tickConnecting() {
    if (_wifi.status() == hal::WifiStatus::Connected) {
        transitionTo(State::Connected);
        return;
    }
    if (_clock.millis() - _connectStartMs > WIFI_CONNECT_TIMEOUT_MS) {
        _wifi.disconnect();
        enterConfigMode();
    }
}

void WifiManager::tickConnected() {
    if (_wifi.status() != hal::WifiStatus::Connected) {
        _eventBus.publish({events::EventType::WifiDisconnected});
        loadAndConnect();
    }
}

void WifiManager::tickConfigMode() {
    _portal.handle();

    if (!_portal.hasNewCredentials()) return;

    auto creds = _portal.consumeCredentials();
    _storage.putString(STORAGE_KEY_WIFI_SSID, creds.ssid);
    _storage.putString(STORAGE_KEY_WIFI_PASS, creds.password);
    _portal.stop();

    _eventBus.publish({events::EventType::WifiCredentialsSaved});

    _wifi.connect(creds.ssid, creds.password);
    transitionTo(State::Connecting);
}

} // namespace core
