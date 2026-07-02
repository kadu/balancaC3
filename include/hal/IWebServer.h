#pragma once

#include <cstdint>
#include <functional>

namespace hal {

class IWebServer {
public:
    virtual ~IWebServer() = default;

    virtual void begin(uint16_t port) = 0;
    virtual void stop() = 0;
    virtual void handle() = 0;
    virtual void on(const char* path, std::function<void()> handler) = 0;
    virtual void send(uint16_t code, const char* contentType, const char* body) = 0;
};

} // namespace hal
