#include "core/Application.h"
#include "events/EventType.h"

namespace core {

Application::Application(hal::ISerial& serial, hal::IClock& clock, events::EventBus& eventBus)
    : _serial(serial), _clock(clock), _eventBus(eventBus) {}

void Application::setup() {
    _serial.begin(115200);

    _eventBus.subscribe(events::EventType::SystemStarted,      this);
    _eventBus.subscribe(events::EventType::SystemReady,        this);
    _eventBus.subscribe(events::EventType::WifiConnecting,     this);
    _eventBus.subscribe(events::EventType::WifiConnected,      this);
    _eventBus.subscribe(events::EventType::WifiDisconnected,   this);
    _eventBus.subscribe(events::EventType::WifiConfigRequired, this);
    _eventBus.subscribe(events::EventType::WifiCredentialsSaved, this);
    _eventBus.subscribe(events::EventType::WebServerStarted,      this);
    _eventBus.subscribe(events::EventType::WifiCredentialsCleared, this);
    _eventBus.subscribe(events::EventType::DeviceRestart,          this);

    _eventBus.publish({events::EventType::SystemStarted});
}

void Application::loop() {
    _eventBus.dispatch();

    if (!_printing) return;

    uint32_t now = _clock.millis();
    if (now - _lastPrintMs < HELLO_WORLD_INTERVAL) return;

    _lastPrintMs = now;
    _serial.println("Hello World!");
    _helloCount++;

    if (_helloCount >= HELLO_WORLD_COUNT) {
        _printing = false;
        _eventBus.publish({events::EventType::SystemReady});
    }
}

void Application::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::SystemStarted:
            _helloCount  = 0;
            _lastPrintMs = _clock.millis();
            _printing    = true;
            break;

        case events::EventType::SystemReady:
            _serial.println("[System] Ready.");
            break;

        case events::EventType::WifiConnecting:
            _serial.println("[WiFi] Conectando...");
            break;

        case events::EventType::WifiConnected:
            _serial.print("[WiFi] Conectado! IP: ");
            _serial.println(event.payload ? static_cast<const char*>(event.payload) : "?");
            break;

        case events::EventType::WifiDisconnected:
            _serial.println("[WiFi] Conexao perdida. Reconectando...");
            break;

        case events::EventType::WifiConfigRequired:
            _serial.println("[WiFi] Sem configuracao. Rede AP: BalancaC3-Config");
            _serial.println("[WiFi] Acesse http://192.168.4.1 para configurar.");
            break;

        case events::EventType::WifiCredentialsSaved:
            _serial.println("[WiFi] Credenciais salvas. Conectando...");
            break;

        case events::EventType::WebServerStarted:
            _serial.print("[Web] Servidor iniciado em http://");
            _serial.println(event.payload ? static_cast<const char*>(event.payload) : "?");
            _serial.print("[Web] Configuracoes em http://");
            _serial.print(event.payload ? static_cast<const char*>(event.payload) : "?");
            _serial.println("/config");
            break;

        case events::EventType::WifiCredentialsCleared:
            _serial.println("[Config] Credenciais apagadas. Reiniciando...");
            break;

        case events::EventType::DeviceRestart:
            _serial.println("[Config] Reiniciando dispositivo...");
            break;

        default:
            break;
    }
}

} // namespace core
