#pragma once

// Serial
static constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Board: ESP32-C3 Super Mini
static constexpr uint8_t PIN_LED_BUILTIN = 8;

// OLED I2C (SSD1306 128x64)
static constexpr uint8_t PIN_I2C_SDA    = 6;
static constexpr uint8_t PIN_I2C_SCL    = 7;
static constexpr uint8_t OLED_I2C_ADDR  = 0x3C;

// WiFi
static constexpr char     WIFI_AP_SSID[]           = "BalancaC3-Config";
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS  = 15000;

// NVS storage keys
static constexpr char STORAGE_KEY_WIFI_SSID[] = "wifi_ssid";
static constexpr char STORAGE_KEY_WIFI_PASS[] = "wifi_pass";

// WS2812B LED Strip
static constexpr uint8_t  PIN_LED_STRIP           = 5;
static constexpr uint8_t  LED_COUNT               = 8;
static constexpr uint8_t  LED_BRIGHTNESS_DEFAULT  = 128;

// NVS storage keys (LED)
static constexpr char STORAGE_KEY_LED_BRIGHTNESS[] = "led_bright";

// Buttons
static constexpr uint8_t PIN_BUTTON_1 = 9;
static constexpr uint8_t PIN_BUTTON_2 = 10;

// NAU7802 Scale
static constexpr float   SCALE_CAPACITY_KG        = 20.0f;
static constexpr uint8_t SCALE_AVERAGE_SAMPLES     = 4;    // loop: fast display (~50ms at 80 SPS)
static constexpr uint8_t SCALE_CALIBRATION_SAMPLES = 10;   // tare/calibrate: more stable
static constexpr float   SCALE_EMA_ALPHA           = 0.08f; // smoothing factor — lower = smoother
static constexpr float   SCALE_DEADBAND_G          = 1.0f;  // min change (g) to update display
static constexpr float   SCALE_ZERO_SNAP_G         = 2.0f;  // snap to 0 when |weight| < this
static constexpr float   SCALE_DEFAULT_KNOWN_WEIGHT_G = 1000.0f;

// NVS storage keys (Scale)
static constexpr char STORAGE_KEY_SCALE_ZERO[]   = "scale_zero";
static constexpr char STORAGE_KEY_SCALE_FACTOR[] = "scale_factor";

// Buzzer
static constexpr uint8_t  PIN_BUZZER          = 2;
static constexpr uint32_t BUZZER_FREQ_HZ      = 2000;
static constexpr uint32_t BUZZER_BEEP_MS      = 30;
