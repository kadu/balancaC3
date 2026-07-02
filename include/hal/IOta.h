#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>

namespace hal {

class IOta {
public:
    virtual ~IOta() = default;

    // ArduinoOTA — network upload by IP (PlatformIO / Arduino IDE)
    using OtaCallback = std::function<void()>;
    using OtaProgressCallback = std::function<void(uint32_t done, uint32_t total)>;
    using OtaErrorCallback = std::function<void(const char* error)>;

    virtual void beginNetwork(const char* hostname,
                              OtaCallback onStart,
                              OtaProgressCallback onProgress,
                              OtaCallback onSuccess,
                              OtaErrorCallback onError) = 0;
    virtual void handleNetwork() = 0;

    // HTTP OTA — firmware upload from browser
    virtual bool httpBegin(size_t totalSize) = 0;
    virtual size_t httpWrite(const uint8_t* data, size_t len) = 0;
    virtual bool httpEnd() = 0;
    virtual bool httpHasError() = 0;
    virtual const char* httpError() = 0;
};

} // namespace hal
