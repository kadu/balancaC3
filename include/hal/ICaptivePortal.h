#pragma once

namespace hal {

struct WifiCredentials {
    char ssid[33];
    char password[65];
};

class ICaptivePortal {
public:
    virtual ~ICaptivePortal() = default;

    virtual void begin() = 0;
    virtual void handle() = 0;
    virtual void stop() = 0;

    virtual bool            hasNewCredentials() = 0;
    virtual WifiCredentials consumeCredentials() = 0;
};

} // namespace hal
