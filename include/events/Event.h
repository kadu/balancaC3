#pragma once

#include "events/EventType.h"

namespace events {

struct Event {
    EventType type;
    const void* payload = nullptr;
};

} // namespace events
