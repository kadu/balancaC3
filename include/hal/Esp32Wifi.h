#pragma once

#include "hal/IWifi.h"
#include <WString.h>

namespace hal {

class Esp32Wifi final : public IWifi {
public:
    void connect(const char* ssid, const char* password) override;
    void disconnect() override;
    WifiStatus status() override;

    void startAP(const char* ssid) override;
    void stopAP() override;

    void        localIP(char* out, size_t maxLen) override;

    uint8_t     scanNetworks() override;
    const char* scannedSSID(uint8_t index) override;
    int8_t      scannedRSSI(uint8_t index) override;

private:
    String _ssidBuffer;
};

} // namespace hal
