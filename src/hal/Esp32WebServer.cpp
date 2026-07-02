#include "hal/Esp32WebServer.h"

namespace hal {

void Esp32WebServer::begin(uint16_t port) {
    _server.begin(port);
}

void Esp32WebServer::stop() {
    _server.stop();
}

void Esp32WebServer::handle() {
    _server.handleClient();
}

void Esp32WebServer::on(const char* path, std::function<void()> handler) {
    _server.on(path, handler);
}

void Esp32WebServer::send(uint16_t code, const char* contentType, const char* body) {
    _server.send(code, contentType, body);
}

} // namespace hal
