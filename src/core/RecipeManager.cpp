#include "core/RecipeManager.h"
#include "events/EventType.h"

namespace core {

// ── Recipe catalogue ─────────────────────────────────────────────────────────
static const Recipe RECIPES[] = {
    {"Receita 1",  0, 0},   // stub — no target yet
    {"Receita 2",  0, 0},   // stub — no target yet
    {"Receita 3",  0, 0},   // stub — no target yet
    {"Sem Receita", 0, 0},  // sentinel — returns to normal mode
};
static constexpr uint8_t RECIPE_COUNT = sizeof(RECIPES) / sizeof(RECIPES[0]);

// ── RecipeManager ─────────────────────────────────────────────────────────────
RecipeManager::RecipeManager(events::EventBus& eventBus)
    : _eventBus(eventBus) {}

void RecipeManager::begin() {
    _eventBus.subscribe(events::EventType::Button2LongPressed, this);
    _eventBus.subscribe(events::EventType::Button1Pressed,     this);
    _eventBus.subscribe(events::EventType::Button1LongPressed, this);
}

void RecipeManager::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::Button2LongPressed:
            if (_state == State::Idle)  openMenu();
            else                        selectCurrent();
            break;

        case events::EventType::Button1Pressed:
            if (_state == State::Menu) {
                _selectedIndex = (_selectedIndex + 1) % RECIPE_COUNT;
                publishMenu();
            }
            break;

        case events::EventType::Button1LongPressed:
            if (_state == State::Menu) closeMenu();
            break;

        default:
            break;
    }
}

void RecipeManager::openMenu() {
    _state         = State::Menu;
    _selectedIndex = 0;
    publishMenu();
}

void RecipeManager::closeMenu() {
    _state = State::Idle;
    _eventBus.publish({events::EventType::RecipeCancelled});
}

void RecipeManager::selectCurrent() {
    const Recipe& r = RECIPES[_selectedIndex];
    _state = State::Idle;

    if (_selectedIndex == RECIPE_COUNT - 1) {
        // "Sem Receita" — just cancel
        _eventBus.publish({events::EventType::RecipeCancelled});
    } else {
        // Stub recipes — publish selected but no active mode yet
        _eventBus.publish({events::EventType::RecipeSelected, &r});
    }
}

void RecipeManager::publishMenu() {
    _menuPayload = {_selectedIndex, RECIPE_COUNT, RECIPES};
    _eventBus.publish({events::EventType::RecipeMenuOpen, &_menuPayload});
}

} // namespace core
