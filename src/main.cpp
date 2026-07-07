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
#include "hal/Esp32Button.h"
#include "hal/Esp32Scale.h"
#include "events/EventBus.h"
#include "core/Application.h"
#include "core/WifiManager.h"
#include "core/WebApp.h"
#include "core/OtaManager.h"
#include "core/DisplayManager.h"
#include "core/LedManager.h"
#include "core/ButtonManager.h"
#include "core/ScaleManager.h"
#include "core/TimerManager.h"
#include "core/BuzzerManager.h"
#include "hal/Esp32Buzzer.h"
#include "config.h"
#include <Wire.h>

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
static hal::Esp32Button        button1(PIN_BUTTON_1);
static hal::Esp32Button        button2(PIN_BUTTON_2);
static hal::Esp32Scale         scale;
static hal::Esp32Buzzer        buzzer;
static events::EventBus        eventBus;
static core::Application       app(serial, eventBus);
static core::WifiManager       wifiManager(espWifi, storage, portal, espClock, eventBus);
static core::WebApp            webApp(webServer, espWifi, storage, device, eventBus);
static core::OtaManager        otaManager(ota, webServer, device, eventBus);
static core::DisplayManager    displayManager(display, espClock, eventBus);
static core::LedManager        ledManager(leds, espClock, eventBus);
static core::ButtonManager     buttonManager(button1, button2, eventBus);
static core::ScaleManager      scaleManager(scale, storage, espClock, eventBus);
static core::TimerManager      timerManager(espClock, eventBus);
static core::BuzzerManager     buzzerManager(buzzer, espClock, eventBus);

static void i2cScan() {
    Serial.println("[I2C] Scanning...");
    uint8_t found = 0;
    for (uint8_t addr = 1; addr < 127; ++addr) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("[I2C] Device found at 0x%02X", addr);
            if (addr == 0x2A) Serial.print("  <- NAU7802");
            if (addr == 0x3C || addr == 0x3D) Serial.print("  <- SSD1306 OLED");
            Serial.println();
            ++found;
        }
    }
    if (found == 0) Serial.println("[I2C] No devices found. Check wiring and pull-ups.");
    else Serial.printf("[I2C] Scan done — %u device(s) found.\n", found);
}

void setup() {
    // Wait for USB-CDC serial to enumerate on the host (ESP32-C3 native USB)
    Serial.begin(115200);
    uint32_t t = millis();
    while (!Serial && (millis() - t < 3000)) delay(10);

    app.setup();

    // Uncomment to wipe WiFi credentials and force captive portal:
    // storage.remove(STORAGE_KEY_WIFI_SSID);
    // storage.remove(STORAGE_KEY_WIFI_PASS);

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    i2cScan();

    char buf[8] = {};
    uint8_t brightness = LED_BRIGHTNESS_DEFAULT;
    if (storage.getString(STORAGE_KEY_LED_BRIGHTNESS, buf, sizeof(buf))) {
        int v = atoi(buf);
        if (v >= 10 && v <= 255) brightness = static_cast<uint8_t>(v);
    }

    ledManager.begin(brightness);
    displayManager.begin();
    buttonManager.begin();
    scaleManager.begin();

    webApp.setScaleManager(&scaleManager);
    timerManager.begin();
    buzzerManager.begin();
    wifiManager.begin();
    webApp.begin();
    otaManager.begin();
}

void loop() {
    app.loop();
    buttonManager.loop();
    buzzerManager.loop();
    timerManager.loop();
    displayManager.loop();
    ledManager.loop();
    scaleManager.loop();
    wifiManager.loop();
    webApp.loop();
    otaManager.loop();
}
