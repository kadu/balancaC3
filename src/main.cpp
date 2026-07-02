#include <Arduino.h>

#include "hal/Esp32Serial.h"
#include "hal/Esp32Clock.h"
#include "hal/Esp32Wifi.h"
#include "hal/Esp32Storage.h"
#include "hal/Esp32CaptivePortal.h"
#include "hal/Esp32WebServer.h"
#include "hal/Esp32Device.h"
#include "events/EventBus.h"
#include "core/Application.h"
#include "core/WifiManager.h"
#include "core/WebApp.h"

static hal::Esp32Serial        serial;
static hal::Esp32Clock         espClock;
static hal::Esp32Wifi          espWifi;
static hal::Esp32Storage       storage;
static hal::Esp32CaptivePortal portal(espWifi);
static hal::Esp32WebServer     webServer;
static hal::Esp32Device        device;
static events::EventBus        eventBus;
static core::Application       app(serial, espClock, eventBus);
static core::WifiManager       wifiManager(espWifi, storage, portal, espClock, eventBus);
static core::WebApp            webApp(webServer, espWifi, storage, device, eventBus);

void setup() {
    app.setup();
    wifiManager.begin();
    webApp.begin();
}

void loop() {
    app.loop();
    wifiManager.loop();
    webApp.loop();
}
