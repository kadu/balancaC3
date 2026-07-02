#pragma once

#include "events/Event.h"

namespace events {

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void onEvent(const Event& event) = 0;
};

} // namespace events
