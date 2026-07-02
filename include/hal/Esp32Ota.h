#pragma once

#include "hal/IOta.h"

namespace hal {

class Esp32Ota final : public IOta {
public:
    void beginNetwork(const char* hostname,
                      OtaCallback onStart,
                      OtaProgressCallback onProgress,
                      OtaCallback onSuccess,
                      OtaErrorCallback onError) override;
    void handleNetwork() override;

    bool   httpBegin(size_t totalSize) override;
    size_t httpWrite(const uint8_t* data, size_t len) override;
    bool   httpEnd() override;
    bool   httpHasError() override;
    const char* httpError() override;
};

} // namespace hal
