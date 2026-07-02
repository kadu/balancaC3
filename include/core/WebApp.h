#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "hal/IWebServer.h"

namespace core {

class WebApp final : public events::IEventHandler {
public:
    WebApp(hal::IWebServer& server, events::EventBus& eventBus);

    void begin();
    void loop();

    void onEvent(const events::Event& event) override;

private:
    hal::IWebServer&  _server;
    events::EventBus& _eventBus;
    bool              _running = false;

    void startServer(const char* ip);
    void stopServer();
    void handleRoot();
};

} // namespace core
