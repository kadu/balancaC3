#include "events/EventBus.h"
#include <Arduino.h>

namespace events {

void EventBus::subscribe(EventType type, IEventHandler* handler) {
    if (_subscriptionCount >= MAX_HANDLERS) {
        Serial.printf("[EventBus] OVERFLOW: MAX_HANDLERS=%u reached, subscription dropped!\n",
                      MAX_HANDLERS);
        return;
    }
    _subscriptions[_subscriptionCount++] = {type, handler};
}

void EventBus::publish(const Event& event) {
    size_t next = (_queueTail + 1) % MAX_QUEUE_SIZE;
    if (next == _queueHead) return; // queue full — drop event
    _queue[_queueTail] = event;
    _queueTail = next;
}

void EventBus::dispatch() {
    while (_queueHead != _queueTail) {
        const Event& event = _queue[_queueHead];
        _queueHead = (_queueHead + 1) % MAX_QUEUE_SIZE;

        for (size_t i = 0; i < _subscriptionCount; ++i) {
            if (_subscriptions[i].type == event.type) {
                _subscriptions[i].handler->onEvent(event);
            }
        }
    }
}

} // namespace events
