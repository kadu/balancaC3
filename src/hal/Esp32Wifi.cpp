#include "hal/Esp32Wifi.h"
#include <WiFi.h>

namespace hal {

void Esp32Wifi::connect(const char* ssid, const char* password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

void Esp32Wifi::disconnect() {
    WiFi.disconnect(true);
}

WifiStatus Esp32Wifi::status() {
    return WiFi.status() == WL_CONNECTED ? WifiStatus::Connected
                                         : WifiStatus::Disconnected;
}

void Esp32Wifi::startAP(const char* ssid) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid);
}

void Esp32Wifi::stopAP() {
    WiFi.softAPdisconnect(true);
}

void Esp32Wifi::localIP(char* out, size_t maxLen) {
    strncpy(out, WiFi.localIP().toString().c_str(), maxLen - 1);
    out[maxLen - 1] = '\0';
}

uint8_t Esp32Wifi::scanNetworks() {
    int n = WiFi.scanNetworks();
    return n < 0 ? 0 : static_cast<uint8_t>(n);
}

const char* Esp32Wifi::scannedSSID(uint8_t index) {
    _ssidBuffer = WiFi.SSID(index);
    return _ssidBuffer.c_str();
}

int8_t Esp32Wifi::scannedRSSI(uint8_t index) {
    return static_cast<int8_t>(WiFi.RSSI(index));
}

} // namespace hal
