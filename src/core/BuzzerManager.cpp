#include "core/BuzzerManager.h"
#include "core/RecipeManager.h"
#include "events/EventType.h"
#include "config.h"

namespace core {

BuzzerManager::BuzzerManager(hal::IBuzzer& buzzer, hal::IClock& clock, events::EventBus& eventBus)
    : _buzzer(buzzer), _clock(clock), _eventBus(eventBus) {}

void BuzzerManager::begin() {
    _buzzer.begin();
    _eventBus.subscribe(events::EventType::Button1Down,    this);
    _eventBus.subscribe(events::EventType::Button2Down,    this);
    _eventBus.subscribe(events::EventType::RecipeStepTick, this);
    _eventBus.subscribe(events::EventType::RecipeFinished, this);
}

void BuzzerManager::loop() {
    uint32_t now = _clock.millis();

    // Finish current beep
    if (_active && now >= _stopAt) {
        _buzzer.stop();
        _active  = false;
        _inGap   = true;
        _nextBeepAt = _stopAt + _gapMs;
    }

    // Start next beep in queue
    if (!_active && _inGap && _beepCount > 0 && now >= _nextBeepAt) {
        _inGap = false;
        uint32_t freq = _freqs[_beepHead];
        uint32_t dur  = _durations[_beepHead];
        _beepHead  = (_beepHead + 1) % MAX_BEEPS;
        _beepCount--;
        _buzzer.beep(freq, dur);
        _stopAt = now + dur;
        _active = true;
    }

    if (_beepCount == 0 && !_active) _inGap = false;
}

void BuzzerManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::Button1Down:
        case events::EventType::Button2Down:
            scheduleBeeps(1, BUZZER_FREQ_HZ, BUZZER_BEEP_MS, 0);
            break;

        case events::EventType::RecipeStepTick: {
            auto* p = static_cast<const RecipeStepPayload*>(event.payload);
            if (p && p->remainingSecs == 1 && p->running) {
                // 3 ascending short beeps — step ending warning
                scheduleBeeps(3, 3000, 60, 80);
            }
            break;
        }

        case events::EventType::RecipeFinished: {
            // 3 descending beeps — recipe complete
            static const uint32_t freqs[] = {2000, 1500, 1000};
            static const uint32_t durs[]  = {120,  120,  200 };
            scheduleSequence(freqs, durs, 3, 100);
            break;
        }

        default:
            break;
    }
}

void BuzzerManager::scheduleBeeps(uint8_t count, uint32_t freqHz, uint32_t durationMs, uint32_t gapMs) {
    _buzzer.stop();
    _active    = false;
    _inGap     = false;
    _beepHead  = 0;
    _beepCount = 0;
    _gapMs     = gapMs > 0 ? gapMs : 80;

    uint8_t n = count < MAX_BEEPS ? count : MAX_BEEPS;
    for (uint8_t i = 0; i < n; ++i) {
        _freqs[i]     = freqHz;
        _durations[i] = durationMs;
    }

    if (n == 0) return;
    // Start first immediately
    _buzzer.beep(freqHz, durationMs);
    _stopAt    = _clock.millis() + durationMs;
    _active    = true;
    _beepHead  = 1;
    _beepCount = n > 1 ? n - 1 : 0;
}

void BuzzerManager::scheduleSequence(const uint32_t* freqs, const uint32_t* durations, uint8_t count, uint32_t gapMs) {
    _buzzer.stop();
    _active    = false;
    _inGap     = false;
    _beepHead  = 0;
    _beepCount = 0;
    _gapMs     = gapMs;

    uint8_t n = count < MAX_BEEPS ? count : MAX_BEEPS;
    for (uint8_t i = 0; i < n; ++i) {
        _freqs[i]     = freqs[i];
        _durations[i] = durations[i];
    }

    if (n == 0) return;
    _buzzer.beep(_freqs[0], _durations[0]);
    _stopAt    = _clock.millis() + _durations[0];
    _active    = true;
    _beepHead  = 1;
    _beepCount = n > 1 ? n - 1 : 0;
}

} // namespace core
