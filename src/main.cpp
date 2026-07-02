#include <Arduino.h>

#include "hal/Esp32Serial.h"
#include "hal/Esp32Clock.h"
#include "events/EventBus.h"
#include "core/Application.h"

static hal::Esp32Serial  serial;
static hal::Esp32Clock   espClock;
static events::EventBus  eventBus;
static core::Application app(serial, espClock, eventBus);

void setup() {
    app.setup();
}

void loop() {
    app.loop();
}
