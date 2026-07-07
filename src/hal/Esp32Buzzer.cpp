#include "hal/Esp32Buzzer.h"
#include "config.h"
#include <Arduino.h>

// LEDC channel dedicated to the buzzer
static constexpr uint8_t  BUZZER_LEDC_CHANNEL    = 0;
static constexpr uint8_t  BUZZER_LEDC_RESOLUTION = 8; // 8-bit duty

namespace hal {

void Esp32Buzzer::begin() {
    ledcSetup(BUZZER_LEDC_CHANNEL, BUZZER_FREQ_HZ, BUZZER_LEDC_RESOLUTION);
    ledcAttachPin(PIN_BUZZER, BUZZER_LEDC_CHANNEL);
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
}

void Esp32Buzzer::beep(uint32_t frequencyHz, uint32_t /*durationMs*/) {
    ledcSetup(BUZZER_LEDC_CHANNEL, frequencyHz, BUZZER_LEDC_RESOLUTION);
    ledcWrite(BUZZER_LEDC_CHANNEL, 128); // 50% duty — BuzzerManager calls stop() after duration
}

void Esp32Buzzer::stop() {
    ledcWrite(BUZZER_LEDC_CHANNEL, 0);
}

} // namespace hal
