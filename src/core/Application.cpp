#include "core/Application.h"
#include "events/EventType.h"
#include <cstdio>

namespace core {

Application::Application(hal::ISerial& serial, events::EventBus& eventBus)
    : _serial(serial), _eventBus(eventBus) {}

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
    _eventBus.subscribe(events::EventType::OtaStarted,             this);
    _eventBus.subscribe(events::EventType::OtaProgress,            this);
    _eventBus.subscribe(events::EventType::OtaSuccess,             this);
    _eventBus.subscribe(events::EventType::OtaError,               this);

    _eventBus.publish({events::EventType::SystemStarted});
}

void Application::loop() {
    _eventBus.dispatch();
}

void Application::onEvent(const events::Event& event) {
    switch (event.type) {
        case events::EventType::SystemStarted:
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

        case events::EventType::OtaStarted:
            _serial.println("[OTA] Atualizacao iniciada...");
            break;

        case events::EventType::OtaProgress: {
            auto* p = static_cast<const uint8_t*>(event.payload);
            if (p) {
                _serial.print("[OTA] Progresso: ");
                char buf[8];
                snprintf(buf, sizeof(buf), "%u%%", *p);
                _serial.println(buf);
            }
            break;
        }

        case events::EventType::OtaSuccess:
            _serial.println("[OTA] Firmware gravado com sucesso! Reiniciando...");
            break;

        case events::EventType::OtaError:
            _serial.print("[OTA] Erro: ");
            _serial.println(event.payload ? static_cast<const char*>(event.payload) : "desconhecido");
            break;

        default:
            break;
    }
}

} // namespace core
