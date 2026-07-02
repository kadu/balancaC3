#pragma once

// Serial
static constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Board: ESP32-C3 Super Mini
static constexpr uint8_t PIN_LED_BUILTIN = 8;

// WiFi
static constexpr char     WIFI_AP_SSID[]           = "BalancaC3-Config";
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS  = 15000;

// NVS storage keys
static constexpr char STORAGE_KEY_WIFI_SSID[] = "wifi_ssid";
static constexpr char STORAGE_KEY_WIFI_PASS[] = "wifi_pass";
