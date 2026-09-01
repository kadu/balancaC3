#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include "core/RecipeStorage.h"
#include "hal/IClock.h"
#include <cstdint>

namespace core {

struct Recipe {
    uint16_t id;
    char     name[48];
    uint16_t targetGrams;
    uint16_t durationSecs;
};

struct RecipeStep {
    char     type[24];
    uint16_t water;
    uint16_t duration;
    char     detail[64];
};

struct RecipeMenuPayload {
    uint8_t       selectedIndex;
    uint8_t       itemCount;
    const Recipe* items;
    bool          confirmed;
};

struct RecipeStepPayload {
    uint8_t  stepIndex;
    uint8_t  totalSteps;
    char     stepType[24];
    uint16_t stepWater;
    uint16_t cumulativeWater;
    uint16_t stepDurationSecs;
    uint32_t totalElapsedSecs;  // total recipe time — always counting up
    uint32_t remainingSecs;     // time left in current step
    bool     running;
};

static constexpr uint8_t RECIPE_MENU_MAX  = 16;
static constexpr uint8_t RECIPE_STEPS_MAX = 24;
static constexpr float   WEIGHT_TRIGGER_G = 1.5f; // weight delta to start step timer

class RecipeManager final : public events::IEventHandler {
public:
    RecipeManager(RecipeStorage& storage, hal::IClock& clock, events::EventBus& eventBus);

    void begin();
    void loop();
    void onEvent(const events::Event& event) override;

    bool isMenuOpen()   const { return _state == State::Menu;   }
    bool isRecipeActive() const { return _state == State::Active; }

    // Web control. The device menu and the web drive the same state machine, so
    // a recipe started from either source shows up on both.
    bool startRecipe(uint16_t id);   // id == 0 cancels; false if load failed
    void cancelRecipe();

    // Read-only state for the web UI — polled instead of mirrored from events,
    // so the values are never a tick behind.
    uint16_t activeRecipeId()    const { return _activeRecipeId; }
    uint8_t  currentStepIndex()  const { return _currentStep; }
    uint8_t  stepCount()         const { return _stepCount; }
    uint32_t stepElapsedSecs()   const { return _stepElapsedMs / 1000; }
    uint32_t stepRemainingSecs() const;
    uint32_t totalElapsedSecs()  const { return _totalElapsedMs / 1000; }
    bool     isStepRunning()     const { return _stepRunning; }

private:
    enum class State : uint8_t { Idle, Menu, Active };

    RecipeStorage&    _storage;
    hal::IClock&      _clock;
    events::EventBus& _eventBus;
    State             _state         = State::Idle;
    uint8_t           _selectedIndex = 0;
    uint16_t          _activeRecipeId = 0;
    uint8_t           _itemCount     = 0;
    bool              _confirmed     = false;

    // Menu items
    Recipe            _menuItems[RECIPE_MENU_MAX + 1];
    RecipeMenuPayload _menuPayload{};

    // Active recipe steps
    char          _recipeName[48]        = {};
    RecipeStep    _steps[RECIPE_STEPS_MAX];
    uint8_t       _stepCount             = 0;
    uint8_t       _currentStep           = 0;
    bool          _stepRunning           = false;
    float         _weightAtStepStart     = 0.0f;
    float         _lastWeight            = 0.0f;
    uint32_t      _stepStartMs           = 0;   // millis when current step started running
    uint32_t      _stepElapsedMs         = 0;   // elapsed within current step (for duration check)
    uint32_t      _totalElapsedMs        = 0;   // total recipe elapsed (never resets)
    uint32_t      _recipeStartMs         = 0;   // millis when first step started
    uint32_t      _lastTickSec           = 0;

    RecipeStepPayload _stepPayload{};

    void loadMenu();
    void openMenu();
    void closeMenu();
    void selectCurrent();
    void publishMenu();

    void activateRecipe(uint16_t id);
    void startStep(uint8_t idx);
    void tickActive();
    void publishStepTick();
    void advanceStep();
};

} // namespace core
