#include "hal/Esp32Ota.h"
#include <ArduinoOTA.h>
#include <Update.h>

namespace hal {

void Esp32Ota::beginNetwork(const char* hostname,
                             OtaCallback onStart,
                             OtaProgressCallback onProgress,
                             OtaCallback onSuccess,
                             OtaErrorCallback onError) {
    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([onStart]() { if (onStart) onStart(); });

    ArduinoOTA.onProgress([onProgress](unsigned int done, unsigned int total) {
        if (onProgress) onProgress(done, total);
    });

    ArduinoOTA.onEnd([onSuccess]() { if (onSuccess) onSuccess(); });

    ArduinoOTA.onError([onError](ota_error_t err) {
        if (!onError) return;
        switch (err) {
            case OTA_AUTH_ERROR:    onError("Auth failed");      break;
            case OTA_BEGIN_ERROR:   onError("Begin failed");     break;
            case OTA_CONNECT_ERROR: onError("Connect failed");   break;
            case OTA_RECEIVE_ERROR: onError("Receive failed");   break;
            case OTA_END_ERROR:     onError("End failed");       break;
            default:                onError("Unknown error");    break;
        }
    });

    ArduinoOTA.begin();
}

void Esp32Ota::handleNetwork() {
    ArduinoOTA.handle();
}

bool Esp32Ota::httpBegin(size_t totalSize) {
    return Update.begin(totalSize == 0 ? UPDATE_SIZE_UNKNOWN : totalSize);
}

size_t Esp32Ota::httpWrite(const uint8_t* data, size_t len) {
    return Update.write(const_cast<uint8_t*>(data), len);
}

bool Esp32Ota::httpEnd() {
    return Update.end(true);
}

bool Esp32Ota::httpHasError() {
    return Update.hasError();
}

const char* Esp32Ota::httpError() {
    return Update.errorString();
}

} // namespace hal
