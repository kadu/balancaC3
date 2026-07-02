#pragma once

#include "hal/IWebServer.h"
#include <WebServer.h>

namespace hal {

class Esp32WebServer final : public IWebServer {
public:
    void begin(uint16_t port) override;
    void stop() override;
    void handle() override;

    void   on(const char* path, std::function<void()> handler) override;
    void   onUpload(const char* path,
                    std::function<void()> completionHandler,
                    std::function<void(UploadStatus, const uint8_t*, size_t)> uploadHandler) override;
    void   send(uint16_t code, const char* contentType, const char* body) override;
    String arg(const char* name) override;

private:
    WebServer _server{80};
};

} // namespace hal
