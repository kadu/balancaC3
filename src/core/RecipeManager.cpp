#include <Arduino.h>
#include <ArduinoJson.h>
#include "core/RecipeManager.h"
#include "core/ScaleManager.h"
#include "events/EventType.h"
#include <cstring>

namespace core {

RecipeManager::RecipeManager(RecipeStorage& storage, hal::IClock& clock, events::EventBus& eventBus)
    : _storage(storage), _clock(clock), _eventBus(eventBus) {}

void RecipeManager::begin() {
    _eventBus.subscribe(events::EventType::Button2LongPressed, this);
    _eventBus.subscribe(events::EventType::Button2Pressed,     this);
    _eventBus.subscribe(events::EventType::Button1Pressed,     this);
    _eventBus.subscribe(events::EventType::Button1LongPressed, this);
    _eventBus.subscribe(events::EventType::WeightUpdated,      this);
}

void RecipeManager::loop() {
    if (_state == State::Active) tickActive();
}

void RecipeManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::Button2Pressed:
            // When confirmed, a simple click also activates (in case long press
            // releases early and is registered as a click instead)
            if (_state == State::Menu && _confirmed) selectCurrent();
            break;

        case events::EventType::Button2LongPressed:
            if      (_state == State::Idle)   openMenu();
            else if (_state == State::Menu) {
                if (!_confirmed) {
                    _confirmed = true;
                    publishMenu();
                } else {
                    selectCurrent();
                }
            }
            else if (_state == State::Active) {
                // Long press exits recipe mode
                cancelRecipe();
            }
            break;

        case events::EventType::Button1Pressed:
            if (_state == State::Menu) {
                if (_confirmed) {
                    // Cancel confirmation — stay on same item
                    _confirmed = false;
                } else {
                    _selectedIndex = (_selectedIndex + 1) % _itemCount;
                }
                publishMenu();
            }
            break;

        case events::EventType::Button1LongPressed:
            if (_state == State::Menu) closeMenu();
            break;

        case events::EventType::WeightUpdated: {
            if (_state != State::Active) break;
            auto* p = static_cast<const core::WeightPayload*>(event.payload);
            if (p) _lastWeight = p->grams;

            // Detect weight change to trigger step timer
            if (!_stepRunning) {
                float delta = _lastWeight - _weightAtStepStart;
                if (delta < 0) delta = -delta;
                if (delta >= WEIGHT_TRIGGER_G) {
                    _stepRunning  = true;
                    _stepStartMs  = _clock.millis();
                    _stepElapsedMs = 0;
                    if (_recipeStartMs == 0) _recipeStartMs = _stepStartMs;
                    publishStepTick();
                }
            }
            break;
        }

        default:
            break;
    }
}

// ── Menu ─────────────────────────────────────────────────────────────────────
void RecipeManager::loadMenu() {
    _itemCount = 0;
    String json = _storage.listRecipes();

    if (json.length() > 2) {
        DynamicJsonDocument doc(2048);
        if (deserializeJson(doc, json) == DeserializationError::Ok) {
            for (JsonObject obj : doc.as<JsonArray>()) {
                if (_itemCount >= RECIPE_MENU_MAX) break;
                Recipe& r = _menuItems[_itemCount];
                r.id           = obj["id"]        | 0;
                r.targetGrams  = obj["waterTotal"] | 0;
                r.durationSecs = obj["totalSecs"]  | 0;
                const char* t  = obj["title"]      | "Sem nome";
                strncpy(r.name, t, sizeof(r.name) - 1);
                r.name[sizeof(r.name) - 1] = '\0';
                _itemCount++;
            }
        }
    }

    Recipe& last = _menuItems[_itemCount];
    last.id           = 0;
    last.targetGrams  = 0;
    last.durationSecs = 0;
    strncpy(last.name, "Sem Receita", sizeof(last.name) - 1);
    last.name[sizeof(last.name) - 1] = '\0';
    _itemCount++;
}

void RecipeManager::openMenu() {
    loadMenu();
    _state         = State::Menu;
    _selectedIndex = 0;
    _confirmed     = false;
    publishMenu();
}

void RecipeManager::closeMenu() {
    cancelRecipe();
}

// ── Web control ──────────────────────────────────────────────────────────────
bool RecipeManager::startRecipe(uint16_t id) {
    if (id == 0) { cancelRecipe(); return true; }
    activateRecipe(id);
    return _state == State::Active;
}

void RecipeManager::cancelRecipe() {
    _state          = State::Idle;
    _activeRecipeId = 0;
    _eventBus.publish({events::EventType::RecipeCancelled});
}

uint32_t RecipeManager::stepRemainingSecs() const {
    if (_state != State::Active || _currentStep >= _stepCount) return 0;
    uint32_t dur = _steps[_currentStep].duration;
    uint32_t el  = _stepElapsedMs / 1000;
    return (dur > el) ? dur - el : 0;
}

void RecipeManager::selectCurrent() {
    const Recipe& r = _menuItems[_selectedIndex];
    _state = State::Idle;
    if (r.id == 0) cancelRecipe();
    else           activateRecipe(r.id);
}

void RecipeManager::publishMenu() {
    _menuPayload = {_selectedIndex, _itemCount, _menuItems, _confirmed};
    _eventBus.publish({events::EventType::RecipeMenuOpen, &_menuPayload});
}

// ── Active mode ───────────────────────────────────────────────────────────────
void RecipeManager::activateRecipe(uint16_t id) {
    String json = _storage.loadRecipe(id);
    if (json.isEmpty()) { cancelRecipe(); return; }

    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, json) != DeserializationError::Ok) {
        cancelRecipe(); return;
    }

    const char* title = doc["title"] | "Receita";
    strncpy(_recipeName, title, sizeof(_recipeName) - 1);
    _recipeName[sizeof(_recipeName) - 1] = '\0';

    _stepCount = 0;
    for (JsonObject s : doc["steps"].as<JsonArray>()) {
        if (_stepCount >= RECIPE_STEPS_MAX) break;
        RecipeStep& rs = _steps[_stepCount];
        const char* t  = s["type"]   | "Despejo";
        const char* d  = s["detail"] | "";
        strncpy(rs.type,   t, sizeof(rs.type)   - 1); rs.type[sizeof(rs.type)   - 1] = '\0';
        strncpy(rs.detail, d, sizeof(rs.detail) - 1); rs.detail[sizeof(rs.detail) - 1] = '\0';
        rs.water    = s["water"]    | 0;
        rs.duration = s["duration"] | 0;
        _stepCount++;
    }

    if (_stepCount == 0) { cancelRecipe(); return; }

    _state           = State::Active;
    _activeRecipeId  = id;
    _lastWeight      = 0.0f;
    _totalElapsedMs  = 0;
    _recipeStartMs   = 0;
    startStep(0);
    _eventBus.publish({events::EventType::RecipeSelected, nullptr});
}

void RecipeManager::startStep(uint8_t idx) {
    _currentStep       = idx;
    _weightAtStepStart = _lastWeight;
    _stepElapsedMs     = 0;
    _lastTickSec       = _totalElapsedMs / 1000; // avoid false tick on entry

    // "Aguardar" starts immediately — no weight trigger needed
    if (strncmp(_steps[idx].type, "Aguardar", 8) == 0) {
        _stepRunning = true;
        _stepStartMs = _clock.millis();
        if (_recipeStartMs == 0) _recipeStartMs = _stepStartMs;
    } else {
        _stepRunning = false;
        _stepStartMs = 0;
    }

    publishStepTick();
    _eventBus.publish({events::EventType::RecipeStepStarted, &_stepPayload});
}

void RecipeManager::tickActive() {
    uint32_t now = _clock.millis();

    // Update total elapsed (counts from first step start)
    if (_recipeStartMs > 0) _totalElapsedMs = now - _recipeStartMs;

    if (!_stepRunning) return;

    _stepElapsedMs = now - _stepStartMs;
    uint32_t totalSec = _totalElapsedMs / 1000;

    if (totalSec == _lastTickSec) return;
    _lastTickSec = totalSec;

    publishStepTick();

    uint16_t dur = _steps[_currentStep].duration;
    if (dur > 0 && _stepElapsedMs / 1000 >= dur) advanceStep();
}

void RecipeManager::publishStepTick() {
    const RecipeStep& s = _steps[_currentStep];
    uint32_t stepSec = _stepElapsedMs / 1000;
    uint32_t dur     = s.duration;
    uint32_t rem     = (dur > 0 && stepSec < dur) ? dur - stepSec : 0;

    // Compute cumulative water up to this step
    uint16_t cumWater = 0;
    for (uint8_t i = 0; i <= _currentStep; ++i) cumWater += _steps[i].water;

    strncpy(_stepPayload.stepType, s.type, sizeof(_stepPayload.stepType) - 1);
    _stepPayload.stepType[sizeof(_stepPayload.stepType) - 1] = '\0';
    _stepPayload.stepIndex         = _currentStep;
    _stepPayload.totalSteps        = _stepCount;
    _stepPayload.stepWater         = s.water;
    _stepPayload.cumulativeWater   = cumWater;
    _stepPayload.stepDurationSecs  = dur;
    _stepPayload.totalElapsedSecs  = _totalElapsedMs / 1000;
    _stepPayload.remainingSecs     = rem;
    _stepPayload.running           = _stepRunning;

    _eventBus.publish({events::EventType::RecipeStepTick, &_stepPayload});
}

void RecipeManager::advanceStep() {
    _eventBus.publish({events::EventType::RecipeStepCompleted, &_stepPayload});

    uint8_t next = _currentStep + 1;
    if (next >= _stepCount) {
        _state          = State::Idle;
        _activeRecipeId = 0;
        _eventBus.publish({events::EventType::RecipeFinished});
    } else {
        startStep(next);
    }
}

} // namespace core
