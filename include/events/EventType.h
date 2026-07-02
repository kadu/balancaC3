#pragma once

#include <cstdint>

namespace events {

enum class EventType : uint8_t {
    SystemStarted,
    SystemReady,
};

} // namespace events
