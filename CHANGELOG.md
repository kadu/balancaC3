# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.12.0] - 2026-07-04

### Added
- Botão "Tarar" na tela inicial da interface web (chama `/scale/tare`, confirma com mensagem de 2s)

### Changed
- Link de Configurações movido para o cabeçalho ao lado do botão de tema, com ícone ⚙ e borda arredondada
- Card do peso simplificado: apenas botão Tarar centralizado

## [0.11.0] - 2026-07-04

### Added
- Botão 2 executa tara (zera a balança) ao ser pressionado

## [0.10.0] - 2026-07-04

### Changed
- Botão 1 pisca **verde** ao ser pressionado (era ciano)
- Botão 2 pisca **vermelho** ao ser pressionado (era ciano)
- `LedManager`: cor do flash parametrizada via `_flashColor` por evento

## [0.9.0] - 2026-07-04

### Changed
- NAU7802: taxa de amostragem 10 SPS → 80 SPS
- Amostras por leitura no loop: 10 → 4 (~50ms por publicação, ~20Hz)
- Tara e calibração mantêm 10 amostras para maior precisão (`SCALE_CALIBRATION_SAMPLES`)
- Polling web: 500ms → 200ms

### Added
- Filtro EMA (`SCALE_EMA_ALPHA=0.15`) para suavizar variações de ruído do ADC
- Deadband (`SCALE_DEADBAND_G=0.3g`) — display só atualiza se mudança superar o limiar
- EMA resetado ao tarar para evitar salto inicial

## [0.8.0] - 2026-07-04

### Added
- HAL layer: `IScale` + `Esp32Scale` (NAU7802, 10 SPS, ganho 128×, HW I2C)
- Core `ScaleManager`: acumulador não-bloqueante de amostras, calibração de dois passos, persistência em NVS
- Endpoints `/scale/weight` (JSON com grams, raw, calibrated, ready), `/scale/tare`, `/scale/calibrate`
- Tela inicial com display de peso em tempo real (polling 500ms): peso em g/kg se calibrado, valor raw se não calibrado
- Seção "Balança" em `/config`: tarar, campo de peso conhecido e calibração em um clique
- `DisplayManager` atualizado: mostra IP + peso em tempo real quando conectado; "Sem calibração" quando não calibrado
- `FIOS_NAU.md`: documentação de fiação do sensor
- `EventBus::MAX_HANDLERS` expandido para 64 com aviso no serial em caso de overflow
- OLED migrado de SW I2C para HW I2C para compartilhar `Wire` com o NAU7802
- `Wire.begin()` centralizado em `main.cpp`; scan I2C no boot com identificação de dispositivos
- Rotas de `WebApp` e `OtaManager` registradas apenas uma vez (fix de re-registro a cada reconexão WiFi)
- Novos eventos: `WeightUpdated`, `ScaleCalibrated`, `ScaleTared`
- `*.code-workspace` adicionado ao `.gitignore`

### Fixed
- `ERR_CONNECTION_REFUSED` causado por overflow silencioso do `EventBus` (39 handlers vs MAX=32)
- Handlers HTTP duplicados a cada reconexão WiFi travavam o WebServer

## [0.7.0] - 2026-07-01

### Added
- HAL layer: `IButton` + `Esp32Button` (OneButton v2, active-low com pull-up interno)
- Static trampolines para compatibilidade com a API `void*(void*)` do OneButton
- Core `ButtonManager`: registra callbacks e publica `Button1Pressed` / `Button2Pressed`
- `LedManager`: novo estado `ButtonFlash` — flash ciano (0, 220, 255) de 80ms ao pressionar, retorna ao estado anterior sem interromper OTA
- Pinos `PIN_BUTTON_1=9`, `PIN_BUTTON_2=10` em `config.h`
- Novos eventos: `Button1Pressed`, `Button2Pressed`
- `mathertel/OneButton @ ^2.6.1` adicionado ao `platformio.ini`

## [0.6.0] - 2026-07-01

### Added
- HAL layer: `ILedStrip` + `Esp32LedStrip` (FastLED, WS2812B GRB, GPIO 5, 8 LEDs)
- Core `LedManager`: máquina de estados não-bloqueante com animações por evento
  - `SystemStarted` → branco sólido 800ms
  - `WifiConnecting` / `WifiDisconnected` → pisca verde 500ms
  - `WifiConnected` → verde sólido 3s → fadeout suave 1.5s
  - `WifiConfigRequired` → respiração amarela contínua (sine wave 3s)
  - `OtaProgress` → barra de progresso azul proporcional ao %
  - `OtaSuccess` → azul sólido 1s → apaga
  - `OtaError` → 3 flashes vermelhos → apaga
- Slider de brilho (10–255) na página `/config` → seção LEDs
- Endpoints `GET /config/led` e `POST /config/led` (brightness persistido em NVS)
- Brilho aplicado instantaneamente via evento `LedBrightnessChanged` sem reiniciar
- `fastled/FastLED @ ^3.9.7` adicionado ao `platformio.ini`
- Constantes `PIN_LED_STRIP=5`, `LED_COUNT=8`, `LED_BRIGHTNESS_DEFAULT=128` em `config.h`

## [0.5.0] - 2026-07-01

### Added
- HAL layer: `IDisplay` + `Esp32Display` (U8g2, SSD1306 128x64 I2C — SDA=8, SCL=9)
- Core `DisplayManager`: ouve eventos WiFi e atualiza o display
  - `WifiConnecting` / `WifiDisconnected` → "Conectando..."
  - `WifiConnected` → "WiFi / Conectado / `<IP>`"
  - `WifiConfigRequired` → "Modo configuracao / BalancaC3-Config / 192.168.4.1"
- Dependência `olikraus/U8g2 @ ^2.35.19` adicionada ao `platformio.ini`
- Pinos I2C definidos em `config.h` (`PIN_I2C_SDA=8`, `PIN_I2C_SCL=9`, `OLED_I2C_ADDR=0x3C`)

## [0.4.0] - 2026-07-01

### Added
- HAL layer: `IOta` + `Esp32Ota` (ArduinoOTA para upload por IP + `Update.h` para upload HTTP)
- `IWebServer::onUpload()` para suporte a upload multipart no browser
- Core `OtaManager`: inicia ArduinoOTA ao conectar, registra rotas `/update` (GET + POST)
- Página `/update` com drag-and-drop de `.bin`, barra de progresso em tempo real e tema claro/escuro
- Botão "Atualizar Firmware (OTA)" na página `/config`
- Serial imprime progresso OTA por percentual e resultado final
- Novos eventos: `OtaStarted`, `OtaProgress` (payload: uint8_t %), `OtaSuccess`, `OtaError`
- `EventBus::MAX_HANDLERS` expandido de 16 para 32

### Removed
- Hello World impresso na serial ao boot
- Dependência de `IClock` removida de `Application` (não mais necessária)

## [0.3.0] - 2026-07-01

### Added
- HAL layer: `IDevice` + `Esp32Device` (wraps `ESP.restart()`)
- `WebApp`: página `/config` com seção WiFi e seção Dispositivo
- Endpoint `GET /config/ssid` — devolve o SSID salvo no NVS
- Endpoint `POST /config/wifi` — salva novas credenciais e reinicia
- Endpoint `POST /config/restart` — reinicia o dispositivo
- Endpoint `POST /config/reset` — apaga credenciais NVS e reinicia (abre portal captive no próximo boot)
- Rede atualmente configurada destacada na lista com borda, fundo e badge "✓ conectada"
- Reinício com delay de 800 ms para garantir entrega da resposta HTTP ao browser
- Serial imprime URL de configurações ao subir o servidor
- Novos eventos: `WifiCredentialsCleared`, `DeviceRestart`

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
