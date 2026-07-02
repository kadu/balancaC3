#pragma once

#include "hal/IWebServer.h"
#include <WebServer.h>

namespace hal {

class Esp32WebServer final : public IWebServer {
public:
    void begin(uint16_t port) override;
    void stop() override;
    void handle() override;
    void on(const char* path, std::function<void()> handler) override;
    void send(uint16_t code, const char* contentType, const char* body) override;

private:
    WebServer _server{80};
};

} // namespace hal
