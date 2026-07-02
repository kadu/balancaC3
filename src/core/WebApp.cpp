#include "core/WebApp.h"
#include "events/EventType.h"

static const char ROOT_HTML[] = R"html(
<!DOCTYPE html><html><head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>BalancaC3</title>
<style>
body{font-family:sans-serif;display:flex;justify-content:center;align-items:center;min-height:100vh;margin:0;background:#f0f2f5}
.box{text-align:center;background:#fff;border-radius:12px;padding:2em 2.5em;box-shadow:0 2px 8px rgba(0,0,0,.1)}
h1{margin:0 0 .3em;color:#1a1a2e}
p{color:#666;margin:0}
</style>
</head><body>
<div class="box"><h1>Hello World!</h1><p>BalancaC3 online</p></div>
</body></html>
)html";

namespace core {

WebApp::WebApp(hal::IWebServer& server, events::EventBus& eventBus)
    : _server(server), _eventBus(eventBus) {}

void WebApp::begin() {
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
}

void WebApp::loop() {
    if (_running) _server.handle();
}

void WebApp::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::WifiConnected:
            startServer(static_cast<const char*>(event.payload));
            break;
        case events::EventType::WifiConfigRequired:
        case events::EventType::WifiDisconnected:
            stopServer();
            break;
        default:
            break;
    }
}

void WebApp::startServer(const char* ip) {
    _server.on("/", [this]() { handleRoot(); });
    _server.begin(80);
    _running = true;
    _eventBus.publish({events::EventType::WebServerStarted, ip});
}

void WebApp::stopServer() {
    if (!_running) return;
    _server.stop();
    _running = false;
}

void WebApp::handleRoot() {
    _server.send(200, "text/html", ROOT_HTML);
}

} // namespace core
