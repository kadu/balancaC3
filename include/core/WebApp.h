#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IWebServer.h"
#include "hal/IStorage.h"
#include "hal/IDevice.h"
#include "hal/IWifi.h"

namespace core {

// Forward declaration to avoid circular include
class ScaleManager;

class WebApp final : public events::IEventHandler {
public:
    WebApp(hal::IWebServer& server, hal::IWifi& wifi, hal::IStorage& storage,
           hal::IDevice& device, events::EventBus& eventBus);

    void setScaleManager(ScaleManager* scale) { _scale = scale; }

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

private:
    hal::IWebServer&  _server;
    hal::IWifi&       _wifi;
    hal::IStorage&    _storage;
    hal::IDevice&     _device;
    events::EventBus& _eventBus;
    ScaleManager*     _scale          = nullptr;
    bool              _running          = false;
    bool              _routesRegistered = false;
    bool              _pendingRestart   = false;
    uint32_t          _restartAt        = 0;

    void startServer(const char* ip);
    void stopServer();
    void registerRoutes();

    void handleRoot();
    void handleConfig();
    void handleConfigWifi();
    void handleConfigRestart();
    void handleConfigReset();
    void handleNetworks();
    void handleCurrentSsid();
    void handleConfigLedGet();
    void handleConfigLedSet();
    void handleConfigLedPreview();
    void handleConfigLedPreviewStop();
    void handleScaleWeight();
    void handleScaleTare();
    void handleScaleCalibrateStep1();
    void handleScaleCalibrateStep2();
    void handleScaleFilterGet();
    void handleScaleFilterSet();
};

} // namespace core
