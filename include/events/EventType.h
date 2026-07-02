#pragma once

#include <cstdint>

namespace events {

enum class EventType : uint8_t {
    SystemStarted,
    SystemReady,
    WifiConnecting,
    WifiConnected,
    WifiDisconnected,
    WifiConfigRequired,
    WifiCredentialsSaved,
    WebServerStarted,
    DeviceRestart,
    WifiCredentialsCleared,
    OtaStarted,
    OtaProgress,
    OtaSuccess,
    OtaError,
};

} // namespace events
