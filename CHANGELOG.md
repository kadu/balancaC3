# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Jingle de inicialização: arpejo ascendente C-Mi-Sol (523→659→784 Hz) tocado durante o logo de boot
- `SystemReady` publicado ao fim do `setup()` em `main.cpp`
- `BuzzerManager` subscreve `SystemReady` e dispara a sequência de boas-vindas

### Added
- Tela de erro permanente no display quando sensor NAU7802 não é encontrado no boot
- Evento `ScaleNotFound` publicado por `ScaleManager::begin()` em caso de falha de inicialização
- `DisplayManager`: estado `ScaleError` exibe "NAU7802 / nao encontrado / Verifique I2C"

## [1.9.0] - 2026-07-04

### Added
- Animação de conclusão de receita no display OLED (28 frames, 80ms/frame, 4s)
- `coffee_anim.h`: 28 frames PROGMEM convertidos do formato Adafruit para U8g2 MSB-first
- `IDisplay::drawBitmapMSB()` para bitmaps no formato Adafruit/MSB-first
- `DisplayManager`: estado `RecipeFinished` com loop de animação não-bloqueante
- `docs/schematic.svg`: diagrama de ligações vetorial com todos os componentes
- README: imagem do esquemático e seção de ligações detalhada por componente

## [1.8.0] - 2026-07-04

### Added
- LEDs durante receita: barra regressiva azul claro proporcional ao tempo restante da etapa
- Aguardando peso → todos os LEDs em azul escuro; ao iniciar → muda para azul claro
- Buzzer: 3 beeps curtos ascendentes quando falta 1 segundo na etapa
- Buzzer: 3 beeps descendentes (2000→1500→1000Hz) ao finalizar a receita
- `BuzzerManager`: fila de beeps com frequências independentes por slot
- `ILedStrip`: cores `lightBlue` e `darkBlue`

### Fixed
- `EventBus::MAX_HANDLERS` expandido de 64 para 96 — overflow silencioso causava
  `WeightUpdated` do `RecipeManager` ser descartado, impedindo início da contagem regressiva

## [1.7.0] - 2026-07-04

### Added
- Modo de receita ativo no dispositivo: ao selecionar receita, display entra em tela dedicada
- Tela de receita: etapa atual, tipo, timer crescente total (nunca zera), regressivo da etapa, peso atual e água alvo
- Disparo automático do timer ao detectar mudança de peso ≥ 1.5g; etapa "Aguardar" inicia imediatamente
- Avanço automático de etapa ao esgotar o tempo; volta ao modo normal ao concluir todas as etapas
- Menu com confirmação em dois passos: `[2L]` marca `ok?`, botão 1 cancela, segundo `[2L]` ou clique simples ativa
- Timer total acumula tempo de todas as etapas (não reseta entre elas)
- `RecipeManager` carrega receitas do LittleFS ao abrir o menu
- Clique longo botão 2 cancela receita ativa e volta à tela normal
- Novos eventos: `RecipeStepStarted`, `RecipeStepTick`, `RecipeStepCompleted`, `RecipeFinished`

## [1.6.0] - 2026-07-04

### Added
- HAL layer: `IFileSystem` + `Esp32FileSystem` (LittleFS, formatOnFail)
- Core `RecipeStorage`: CRUD de receitas em JSON no LittleFS (`/recipes/<id>.json`)
- Aba **Receitas** (primeira aba em `/config`) com lista, cadastro e edição
- Estrutura de receita: título, moagem, água total, café (g), temperatura, etapas
- Etapas com tipo (Despejo, Flor, Aguardar, Redemoinho, Mexa, Personalizado), água, duração, detalhe
- Sugestão automática de água restante ao adicionar nova etapa
- Campo de água desabilitado e zerado automaticamente ao selecionar "Aguardar"
- Validação ao salvar: impede salvar se água total não foi totalmente distribuída
- Tempo total da receita calculado em tempo real e exibido no formulário
- Lista de receitas exibe água total e tempo total (M:SS) por receita
- `totalSecs` e `waterTotal` incluídos no índice de receitas
- `bblanchon/ArduinoJson @ ^6.21.5` adicionado ao `platformio.ini`
- `-DARDUINOJSON_ENABLE_PROGMEM=0` para compatibilidade com C++17 no ESP32

## [1.5.0] - 2026-07-04

### Added
- Modo de receitas com menu de seleção no display
- `RecipeManager`: máquina de estados Menu/Idle com catálogo de receitas
- Menu ativado por clique longo no botão 2
- Botão 1 navega para baixo (com scroll de 3 itens visíveis e indicadores `^`/`v`)
- Clique longo botão 2 seleciona o item; clique longo botão 1 cancela
- "Sem Receita" volta ao modo de pesagem normal
- Receita 1/2/3 fecham o menu (stub — sem ação ainda)
- `Button2LongPressed` adicionado ao `ButtonManager`
- Novos eventos: `Button2LongPressed`, `RecipeMenuOpen`, `RecipeSelected`, `RecipeCancelled`
- `DisplayManager`: estado `RecipeMenu` com layout de menu e hints de navegação

## [1.4.1] - 2026-07-04

### Added
- Ícone de status do timer ao lado do tempo no display
  - **▶** (triângulo) quando o timer está rodando
  - **⏸** (dois retângulos) quando pausado ou parado
- `IDisplay`: `drawVLine()`, `drawBox()`, `drawTriangle()`
- `DisplayManager` assina `TimerStarted` e `TimerPaused` para atualizar o ícone em tempo real

## [1.4.0] - 2026-07-04

### Added
- Aba Balança: card "Filtro de leitura" com 4 sliders ajustáveis pela interface web
  - Suavização (EMA alpha 1–50%)
  - Sensibilidade / deadband (0.1–5.0g)
  - Zona morta no zero (0.1–10.0g)
  - Velocidade de leitura (1–20 amostras)
- Endpoints `GET /scale/filter` e `POST /scale/filter`
- Configurações do filtro persistidas em NVS
- `ScaleManager::setFilterConfig()` aplica e persiste parâmetros em runtime
- Mensagens de sucesso nas configurações somem automaticamente após 3s

## [1.3.0] - 2026-07-04

### Changed
- Página `/config` reformulada com layout de tabs: Balança, LEDs, WiFi, Dispositivo
- Scan de redes WiFi lazy (só carrega ao abrir a aba WiFi)
- Browser lembra a última aba aberta via `localStorage`
- Aba LEDs: guia de cores em formato de tabela com bolinha animada, status e descrição de cada cor

## [1.2.1] - 2026-07-04

### Fixed
- Display não mostrava mais "Sem calibração" desnecessariamente ao ligar com sensor já calibrado
- `ScaleManager::begin()` publica `ScaleCalibrated` imediatamente se calibração existe no NVS
- `DisplayManager` sincroniza estado de calibração diretamente do `ScaleManager` ao sair do logo splash

## [1.2.0] - 2026-07-04

### Added
- Tela de logo no boot: exibe bitmap 128×64 por 3 segundos antes de iniciar o WiFi
- `logo.h`: bitmap do logo em PROGMEM
- `IDisplay::drawBitmap()` + `Esp32Display::drawBitmap()` usando `drawXBMP` do U8g2
- `DisplayManager`: estado `SplashLogo` bloqueia eventos WiFi durante exibição; ao terminar vai direto para splash de IP se já conectou

## [1.1.0] - 2026-07-04

### Added
- HAL layer: `IBuzzer` + `Esp32Buzzer` (LEDC PWM, GPIO 2, não-bloqueante)
- Core `BuzzerManager`: beep de 30ms a 2kHz ao pressionar qualquer botão (`Button1Down`/`Button2Down`)
- Constantes `PIN_BUZZER=2`, `BUZZER_FREQ_HZ=2000`, `BUZZER_BEEP_MS=30` em `config.h`

## [1.0.0] - 2026-07-04

Primeira versão estável. Balança de café completa com timer, LEDs de status, display OLED, interface web, calibração, OTA e configuração WiFi por portal captive.

### Incluído nesta versão
- Pesagem em tempo real com sensor NAU7802 (célula de carga até 20kg)
- Filtro EMA com deadband para leitura estável
- Timer M:SS com iniciar/pausar/resetar por botões físicos
- Display OLED com splash animado e layout timer + peso
- 8 LEDs WS2812B com animações de status e brilho ajustável
- Feedback imediato nos LEDs ao pressionar botões
- Interface web com peso ao vivo, tara, calibração e configuração
- Portal captive para configuração WiFi sem cabo
- Atualização de firmware OTA pela interface web e por rede
- Persistência de configurações em NVS (WiFi, brilho, calibração)

## [0.16.0] - 2026-07-04

### Changed
- LED acende imediatamente ao **pressionar** o botão (via `attachPress`), apaga ao **soltar** (via `attachClick`)
- `IButton` + `Esp32Button`: novo método `onDown()` usando `attachPress` do OneButton
- `ButtonManager`: publica `Button1Down` / `Button2Down` ao pressionar
- `LedManager`: flash disparado por `Button1Down`/`Button2Down`; apaga em `Button1Pressed`/`Button2Pressed`; fallback de 2s para cliques longos
- Novos eventos: `Button1Down`, `Button2Down`

## [0.15.0] - 2026-07-04

### Added
- Preview ao vivo de brilho dos LEDs na página `/config`
- Botão 💡 ativa preview: LEDs acendem em verde com o brilho do slider
- Mover o slider atualiza a intensidade dos LEDs em tempo real
- Botão 💾 salva no NVS e encerra o preview automaticamente
- Novos eventos: `LedPreviewChanged` (payload: uint8_t brilho), `LedPreviewStopped`
- `LedManager`: estado `Preview` — verde sólido com brilho variável, retorna ao estado anterior ao parar
- Endpoints `POST /config/led/preview` e `POST /config/led/preview/stop`

## [0.14.0] - 2026-07-04

### Added
- `TimerManager`: timer com pausa/retoma (botão 1) e reset por clique longo
- `Button1LongPressed` event publicado pelo `ButtonManager`
- `DisplayManager`: splash de 3s ao conectar WiFi com IP deslizando da direita para o centro
- `DisplayManager`: splash de 2.5s no modo AP com instruções de configuração
- `IDisplay`: `drawStringAt(int16_t x, y)`, `stringWidth()`, `setFontXLarge()`, `displayWidth()`
- `Esp32Display`: fonte XLarge (`u8g2_font_logisoso28_tf`, 28px)
- `DisplayManager::loop()` para animação de scroll não-bloqueante

### Changed
- Layout do display em modo balança: sem traço separador, timer linha 20, peso XLarge linha 56
- `DisplayManager` recebe `IClock` no construtor para controle de timing interno

## [0.13.0] - 2026-07-04

### Changed
- Filtro EMA: alpha 0.15 → 0.08 (mais suavização)
- Deadband: 0.3g → 1.0g
- Snap ao zero: ±0.5g → ±2.0g, aplicado **após** o EMA (absorve ruído residual antes de decidir)
- Display travado em 0.0g com plataforma vazia

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
