#pragma once

#include "events/IEventHandler.h"
#include "events/EventBus.h"
#include <cstdint>

namespace core {

struct Recipe {
    const char* name;
    uint16_t    targetGrams;   // 0 = not configured yet
    uint16_t    durationSecs;  // countdown duration in seconds
};

struct RecipeMenuPayload {
    uint8_t      selectedIndex;
    uint8_t      itemCount;
    const Recipe* items;
};

class RecipeManager final : public events::IEventHandler {
public:
    explicit RecipeManager(events::EventBus& eventBus);

    void begin();
    void onEvent(const events::Event& event) override;

    bool isMenuOpen() const { return _state == State::Menu; }

private:
    enum class State : uint8_t { Idle, Menu };

    events::EventBus& _eventBus;
    State             _state         = State::Idle;
    uint8_t           _selectedIndex = 0;

    RecipeMenuPayload _menuPayload{};

    void openMenu();
    void closeMenu();
    void selectCurrent();
    void publishMenu();
};

} // namespace core
