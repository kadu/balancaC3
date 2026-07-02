# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-06-30

### Added
- PlatformIO project setup for ESP32-C3 Super Mini (`esp32-c3-devkitm-1`)
- HAL layer: `ISerial` interface + `Esp32Serial` implementation
- HAL layer: `IClock` interface + `Esp32Clock` implementation (wraps `millis()`)
- Event infrastructure: `EventBus` (heap-free, fixed-size queue), `IEventHandler`, `Event`, `EventType`
- Core `Application` class implementing event-driven architecture
- Boot sequence: prints "Hello World!" 5× at 1 s non-blocking intervals via `IClock`, then "[System] Ready."
- Serial configured at 115200 baud
