#pragma once

#include "events/Event.h"
#include "events/IEventHandler.h"
#include <array>
#include <cstddef>

namespace events {

static constexpr size_t MAX_HANDLERS = 96;
static constexpr size_t MAX_QUEUE_SIZE = 16;

class EventBus {
public:
    void subscribe(EventType type, IEventHandler* handler);
    void publish(const Event& event);
    void dispatch();

private:
    struct Subscription {
        EventType type;
        IEventHandler* handler = nullptr;
    };

    std::array<Subscription, MAX_HANDLERS> _subscriptions{};
    size_t _subscriptionCount = 0;

    std::array<Event, MAX_QUEUE_SIZE> _queue{};
    size_t _queueHead = 0;
    size_t _queueTail = 0;
};

} // namespace events
