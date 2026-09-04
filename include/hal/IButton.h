#pragma once

#include <functional>
#include <stdint.h>

namespace hal {

using ButtonCallback = std::function<void()>;

class IButton {
public:
    virtual ~IButton() = default;

    virtual void begin() = 0;
    virtual void tick() = 0;

    // Ajusta debounce, a janela de decisao entre clique simples e duplo, e o
    // tempo minimo para o acionamento virar clique longo. Necessario porque
    // botoes mecanicos e modulos touch tem tempos muito diferentes.
    virtual void setTimings(uint16_t debounceMs, uint16_t clickMs, uint16_t longPressMs) = 0;

    // Descarta leituras nos primeiros ms apos o boot, para sensores que precisam
    // de um tempo de estabilizacao antes de a saida significar alguma coisa.
    virtual void ignoreFirstMs(uint16_t ms) = 0;

    virtual void onDown(ButtonCallback cb) = 0;
    virtual void onPress(ButtonCallback cb) = 0;
    virtual void onLongPress(ButtonCallback cb) = 0;
    virtual void onDoubleClick(ButtonCallback cb) = 0;
};

} // namespace hal
