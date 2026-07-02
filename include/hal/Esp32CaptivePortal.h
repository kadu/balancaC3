#pragma once

#include "hal/ICaptivePortal.h"
#include "hal/IWifi.h"
#include <WebServer.h>
#include <DNSServer.h>

namespace hal {

class Esp32CaptivePortal final : public ICaptivePortal {
public:
    explicit Esp32CaptivePortal(IWifi& wifi);

    void begin() override;
    void handle() override;
    void stop() override;

    bool            hasNewCredentials() override;
    WifiCredentials consumeCredentials() override;

private:
    IWifi&     _wifi;
    WebServer  _server{80};
    DNSServer  _dns;

    bool            _hasCredentials = false;
    WifiCredentials _pending{};

    void registerRoutes();
    void handleRoot();
    void handleNetworks();
    void handleSave();
    void redirectToRoot();
};

} // namespace hal
