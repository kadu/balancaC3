#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IOta.h"
#include "hal/IWebServer.h"
#include "hal/IDevice.h"

namespace core {

class OtaManager final : public events::IEventHandler {
public:
    OtaManager(hal::IOta& ota, hal::IWebServer& server,
                hal::IDevice& device, events::EventBus& eventBus);

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

private:
    hal::IOta&        _ota;
    hal::IWebServer&  _server;
    hal::IDevice&     _device;
    events::EventBus& _eventBus;

    bool     _networkActive    = false;
    bool     _routesRegistered = false;
    bool     _pendingRestart   = false;
    uint32_t _restartAt       = 0;
    uint32_t _lastProgressPct = 0;

    void startOta(const char* ip);
    void registerHttpRoute();
    void handleUploadCompletion();
    void handleUploadChunk(hal::UploadStatus status, const uint8_t* data, size_t len);
};

} // namespace core
