#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <WString.h>

namespace hal {

enum class UploadStatus : uint8_t { Start, Write, End, Abort };

class IWebServer {
public:
    virtual ~IWebServer() = default;

    virtual void begin(uint16_t port) = 0;
    virtual void stop() = 0;
    virtual void handle() = 0;

    virtual void   on(const char* path, std::function<void()> handler) = 0;
    virtual void   onUpload(const char* path,
                            std::function<void()> completionHandler,
                            std::function<void(UploadStatus, const uint8_t*, size_t)> uploadHandler) = 0;
    virtual void   send(uint16_t code, const char* contentType, const char* body) = 0;
    virtual String arg(const char* name) = 0;
};

} // namespace hal
