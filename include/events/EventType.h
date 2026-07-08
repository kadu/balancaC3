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
    LedBrightnessChanged,
    Button1Pressed,
    Button2Pressed,
    WeightUpdated,
    ScaleCalibrated,
    ScaleTared,
    TimerStarted,
    TimerPaused,
    TimerResumed,
    TimerReset,
    TimerTick,
    Button1LongPressed,
    LedPreviewChanged,
    LedPreviewStopped,
    Button1Down,
    Button2Down,
    Button2LongPressed,
    RecipeMenuOpen,
    RecipeSelected,
    RecipeCancelled,
    RecipeStepStarted,
    RecipeStepTick,
    RecipeStepCompleted,
    RecipeFinished,
};

} // namespace events
