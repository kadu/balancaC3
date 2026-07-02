# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2026-07-01

### Added
- HAL layer: `IWifi` + `Esp32Wifi` (connect/disconnect/AP/scan/localIP)
- HAL layer: `IStorage` + `Esp32Storage` (NVS via `Preferences.h`)
- HAL layer: `ICaptivePortal` + `Esp32CaptivePortal` (DNS + HTTP server, portal HTML embarcado)
- HAL layer: `IWebServer` + `Esp32WebServer`
- Core `WifiManager`: máquina de estados (Connecting → Connected → ConfigMode) com reconexão automática e timeout de 15 s
- Core `WebApp`: servidor HTTP na porta 80, sobe ao conectar e derruba no modo AP
- Portal captive com tema claro/escuro (detecção automática + alternância manual), campo SSID editável para redes ocultas, listagem de redes com sinal
- Credenciais WiFi persistidas em NVS; reconecta automaticamente no boot
- Eventos: `WifiConnecting`, `WifiConnected` (payload: IP), `WifiDisconnected`, `WifiConfigRequired`, `WifiCredentialsSaved`, `WebServerStarted` (payload: IP)
- Serial imprime IP ao conectar e URL do servidor web ao subir

## [0.1.0] - 2026-06-30

### Added
- PlatformIO project setup for ESP32-C3 Super Mini (`esp32-c3-devkitm-1`)
- HAL layer: `ISerial` interface + `Esp32Serial` implementation
- HAL layer: `IClock` interface + `Esp32Clock` implementation (wraps `millis()`)
- Event infrastructure: `EventBus` (heap-free, fixed-size queue), `IEventHandler`, `Event`, `EventType`
- Core `Application` class implementing event-driven architecture
- Boot sequence: prints "Hello World!" 5× at 1 s non-blocking intervals via `IClock`, then "[System] Ready."
- Serial configured at 115200 baud
