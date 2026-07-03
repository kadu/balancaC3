#include <Arduino.h>

#include "hal/Esp32Serial.h"
#include "hal/Esp32Clock.h"
#include "hal/Esp32Wifi.h"
#include "hal/Esp32Storage.h"
#include "hal/Esp32CaptivePortal.h"
#include "hal/Esp32WebServer.h"
#include "hal/Esp32Device.h"
#include "hal/Esp32Ota.h"
#include "hal/Esp32Display.h"
#include "hal/Esp32LedStrip.h"
#include "events/EventBus.h"
#include "core/Application.h"
#include "core/WifiManager.h"
#include "core/WebApp.h"
#include "core/OtaManager.h"
#include "core/DisplayManager.h"
#include "core/LedManager.h"
#include "config.h"

static hal::Esp32Serial        serial;
static hal::Esp32Clock         espClock;
static hal::Esp32Wifi          espWifi;
static hal::Esp32Storage       storage;
static hal::Esp32CaptivePortal portal(espWifi);
static hal::Esp32WebServer     webServer;
static hal::Esp32Device        device;
static hal::Esp32Ota           ota;
static hal::Esp32Display       display;
static hal::Esp32LedStrip      leds;
static events::EventBus        eventBus;
static core::Application       app(serial, eventBus);
static core::WifiManager       wifiManager(espWifi, storage, portal, espClock, eventBus);
static core::WebApp            webApp(webServer, espWifi, storage, device, eventBus);
static core::OtaManager        otaManager(ota, webServer, device, eventBus);
static core::DisplayManager    displayManager(display, eventBus);
static core::LedManager        ledManager(leds, espClock, eventBus);

void setup() {
    app.setup();

    // Load saved brightness before begin (default if not set)
    char buf[8] = {};
    uint8_t brightness = LED_BRIGHTNESS_DEFAULT;
    if (storage.getString(STORAGE_KEY_LED_BRIGHTNESS, buf, sizeof(buf))) {
        int v = atoi(buf);
        if (v >= 10 && v <= 255) brightness = static_cast<uint8_t>(v);
    }

    ledManager.begin(brightness);
    displayManager.begin();
    wifiManager.begin();
    webApp.begin();
    otaManager.begin();
}

void loop() {
    app.loop();
    ledManager.loop();
    wifiManager.loop();
    webApp.loop();
    otaManager.loop();
}
