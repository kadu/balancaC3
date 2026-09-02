#pragma once

// Versao do firmware — subir ao lancar release (ver CHANGELOG.md).
// A data/hora do build e o hash do git vem de build_info.h, gerado a cada build.
static constexpr char FIRMWARE_VERSION[] = "1.10.0";

// Serial
static constexpr uint32_t SERIAL_BAUD_RATE = 115200;

// Board: ESP32-C3 Super Mini
static constexpr uint8_t PIN_LED_BUILTIN = 8;

// OLED I2C (SSD1306 128x64)
static constexpr uint8_t PIN_I2C_SDA    = 6;
static constexpr uint8_t PIN_I2C_SCL    = 7;
static constexpr uint8_t OLED_I2C_ADDR  = 0x3C;

// WiFi
static constexpr char     WIFI_AP_SSID[]           = "BalancaC3-Config";
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS  = 15000;

// NVS storage keys
static constexpr char STORAGE_KEY_WIFI_SSID[] = "wifi_ssid";
static constexpr char STORAGE_KEY_WIFI_PASS[] = "wifi_pass";

// WS2812B LED Strip
static constexpr uint8_t  PIN_LED_STRIP           = 5;
static constexpr uint8_t  LED_COUNT               = 8;
static constexpr uint8_t  LED_BRIGHTNESS_DEFAULT  = 128;

// NVS storage keys (LED)
static constexpr char STORAGE_KEY_LED_BRIGHTNESS[] = "led_bright";

// Buttons
static constexpr uint8_t PIN_BUTTON_1 = 3;
static constexpr uint8_t PIN_BUTTON_2 = 1;

// Modulos touch TTP223B. De fabrica o pad AHLB fica aberto, o que deixa a saida
// ATIVA EM NIVEL ALTO — o oposto do botao mecanico que ligava o pino ao GND. A
// saida e CMOS push-pull (4mA source / 8mA sink), entao o pull-up interno nao so
// e desnecessario como briga com o pino quando ele puxa para baixo.
// Se em vez do touch voltar um botao mecanico, ambos voltam para true.
static constexpr bool BUTTON_ACTIVE_LOW    = false;
static constexpr bool BUTTON_PULLUP_ACTIVE = false;

// Tempo de estabilizacao apos energizar: o TTP223B passa ~0.5s se auto-calibrando
// com a deteccao desabilitada, e nesse intervalo a saida nao vale nada. Ignorar
// esse periodo evita registrar um toque fantasma no boot.
static constexpr uint16_t BUTTON_STARTUP_MS = 600;

// Tempos do OneButton, ajustados para o touch:
// - DEBOUNCE: a saida do TTP223B ja e digital e limpa (nao ha repique mecanico),
//   e o proprio CI leva ate 220ms para responder em modo de baixo consumo. Os
//   50ms padrao so somavam atraso e engoliam toques curtos.
// - CLICK: janela apos soltar para decidir entre clique simples e duplo. So tem
//   efeito com BUTTON_DOUBLE_CLICK habilitado; sem ele o clique sai na soltura.
// - LONG_PRESS: acima disso o acionamento vira clique longo. Mais alto que o
//   padrao de 800ms porque o dedo fica no sensor mais tempo que num botao.
static constexpr uint16_t BUTTON_DEBOUNCE_MS   = 20;
static constexpr uint16_t BUTTON_CLICK_MS      = 350;
static constexpr uint16_t BUTTON_LONG_PRESS_MS = 1500;

// Habilita o clique duplo. O custo e real: com ele ligado o clique simples so
// dispara BUTTON_CLICK_MS depois de soltar, porque o OneButton precisa esperar
// para saber se vem um segundo toque. Desligue para ter a tara instantanea.
static constexpr bool BUTTON_DOUBLE_CLICK = true;

// LONG_PRESS_MS tem uma copia em LONG_MS no JS da pagina inicial (WebApp.cpp):
// mudar aqui exige mudar la, senao o botao da web para de equivaler ao fisico.

// NAU7802 Scale
static constexpr float   SCALE_CAPACITY_KG        = 20.0f;
static constexpr uint8_t SCALE_AVERAGE_SAMPLES     = 4;    // loop: fast display (~50ms at 80 SPS)
static constexpr uint8_t SCALE_CALIBRATION_SAMPLES = 10;   // tare/calibrate: more stable
static constexpr float   SCALE_EMA_ALPHA           = 0.08f; // smoothing factor — lower = smoother
static constexpr float   SCALE_DEADBAND_G          = 1.0f;  // min change (g) to update display
static constexpr float   SCALE_ZERO_SNAP_G         = 2.0f;  // snap to 0 when |weight| < this
static constexpr float   SCALE_DEFAULT_KNOWN_WEIGHT_G = 1000.0f;

// NVS storage keys (Scale)
static constexpr char STORAGE_KEY_SCALE_ZERO[]   = "scale_zero";
static constexpr char STORAGE_KEY_SCALE_FACTOR[] = "scale_factor";

// Buzzer
static constexpr uint8_t  PIN_BUZZER          = 2;
static constexpr uint32_t BUZZER_FREQ_HZ      = 2000;
static constexpr uint32_t BUZZER_BEEP_MS      = 30;

// NVS storage keys (Scale filter)
static constexpr char STORAGE_KEY_SCALE_EMA[]      = "scale_ema";
static constexpr char STORAGE_KEY_SCALE_DEADBAND[] = "scale_dead";
static constexpr char STORAGE_KEY_SCALE_SNAP[]     = "scale_snap";
static constexpr char STORAGE_KEY_SCALE_SAMPLES[]  = "scale_samp";
