# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

ESP32-C3 Super Mini firmware using PlatformIO + Arduino framework. Target: `esp32-c3-devkitm-1`.

## Commands

```bash
# Build
pio run -e esp32-c3-devkitm-1

# Upload
pio run -e esp32-c3-devkitm-1 --target upload

# Monitor serial (115200 baud)
pio device monitor

# Upload + monitor
pio run -e esp32-c3-devkitm-1 --target upload && pio device monitor

# Clean
pio run --target clean
```

## Architecture

Three strict layers — dependencies only point inward (no layer imports from a layer above it):

```
src/main.cpp          ← Composition root: instantiates HAL + EventBus + Application
│
├── core/             ← Business logic (no Arduino.h, no hardware)
│   └── Application   ← Implements IEventHandler; drives the main state machine
│
├── events/           ← Event infrastructure (no Arduino.h, no hardware)
│   ├── EventBus      ← Static-array queue + subscriber dispatch (heap-free)
│   ├── IEventHandler ← Interface all listeners implement
│   ├── Event         ← Plain struct {EventType, payload*}
│   └── EventType     ← enum class of all system events
│
└── hal/              ← Hardware Abstraction Layer (only place Arduino.h is allowed)
    ├── ISerial       ← Abstract interface
    └── Esp32Serial   ← Concrete Arduino Serial wrapper
```

### Key design rules

- **HAL isolation**: `Arduino.h` is only included in `src/hal/`. Core and Events layers are platform-agnostic.
- **Heap-free**: `EventBus` uses fixed-size `std::array`; no `new`/`delete`/`malloc`. Sizes are in `EventBus.h` (`MAX_HANDLERS`, `MAX_QUEUE_SIZE`).
- **Event-driven loop**: `main.cpp::loop()` calls only `app.loop()`, which calls `eventBus.dispatch()`. All logic is triggered by events.
- **Composition root**: `main.cpp` owns all static instances and wires dependencies via constructor injection.
- **No globals in layers**: All state lives in class members; `main.cpp` static instances are the only globals.

### Adding a new feature

1. Add the `EventType` value to `include/events/EventType.h`.
2. Create a HAL interface in `include/hal/I<Name>.h` and implement it in `include/hal/<Name>.h` + `src/hal/<Name>.cpp`.
3. Create a handler class in `include/core/` that implements `IEventHandler`.
4. Register in `src/main.cpp` (instantiate + `subscribe`).

### Board-specific pins

Defined in `include/config.h`. ESP32-C3 Super Mini built-in LED is GPIO 8.

## Git Flow & Iteration Protocol

This project follows Git Flow. Every feature is developed on a dedicated branch and committed only after explicit user approval.

### Branch model

```
main          ← stable releases only (tagged)
develop       ← integration branch
feature/*     ← one branch per feature
hotfix/*      ← critical fixes branched from main
```

### Mandatory iteration cycle

Each feature follows this exact sequence — **no exceptions**:

1. **Branch**: `git checkout develop && git checkout -b feature/<name>`
2. **Implement**: write code following the architecture rules above
3. **Build + Upload**: run `pio run -e esp32-c3-devkitm-1 --target upload` — must succeed before proceeding. If no device is connected, run build-only (`pio run -e esp32-c3-devkitm-1`) and note the device was absent.
4. **Notify**: inform the user that build+upload succeeded and the firmware is ready to test. **Wait for explicit approval to commit.**
5. **Commit** (only after approval):
   - Update `CHANGELOG.md` under `[Unreleased]` with a concise entry
   - `git add -A && git commit -m "feat(<scope>): <description>"`
6. **Merge**: `git checkout develop && git merge --no-ff feature/<name> && git branch -d feature/<name>`
7. **Next feature**: only start after the previous commit is done and confirmed

### Interruption rule

If a new feature is requested while the current one is **not yet committed**, respond with:

> "A feature `<current>` is in progress and not yet committed. Should I finish and commit it first, or discard the current work and start the new one?"

Do not touch the code until the user decides.

### Commit message format

```
feat(scope): short description      ← new feature
fix(scope): short description       ← bug fix
refactor(scope): short description  ← no behavior change
chore(scope): short description     ← build / config / tooling
```

### CHANGELOG rules

- File: `CHANGELOG.md` at project root, format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
- Every commit that changes behavior must add a line under `[Unreleased]`
- On release (`main` merge + tag): move `[Unreleased]` entries to a new `[x.y.z] - YYYY-MM-DD` section
