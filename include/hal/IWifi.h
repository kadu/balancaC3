#pragma once

#include <cstdint>
#include <cstddef>

namespace hal {

enum class WifiStatus : uint8_t {
    Disconnected,
    Connecting,
    Connected,
};

class IWifi {
public:
    virtual ~IWifi() = default;

    virtual void connect(const char* ssid, const char* password) = 0;
    virtual void disconnect() = 0;
    virtual WifiStatus status() = 0;

    virtual void startAP(const char* ssid) = 0;
    virtual void stopAP() = 0;

    virtual void        localIP(char* out, size_t maxLen) = 0;

    virtual uint8_t     scanNetworks() = 0;
    virtual const char* scannedSSID(uint8_t index) = 0;
    virtual int8_t      scannedRSSI(uint8_t index) = 0;
};

} // namespace hal
