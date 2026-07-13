# BalancaC3

Uma balança de precisão para café, construída em torno do microcontrolador ESP32-C3, pensada para quem leva a sério cada detalhe da extração.

---

## O que é

BalancaC3 é um dispositivo embarcado que combina balança de precisão, timer e conectividade WiFi em um equipamento compacto. Ele foi projetado para acompanhar o processo de preparo do café — seja pour over, espresso, aeropress ou qualquer outro método que dependa de controle de peso e tempo.

---

## O que ele faz

### Pesagem
- Mede o peso em tempo real com resolução de 0,1g
- Display OLED mostra o valor de forma clara e legível
- Atualização rápida para acompanhar o café sendo despejado
- Tara com um toque no botão — zera a medição instantaneamente

### Timer
- Timer integrado no mesmo display do peso
- Inicia e pausa com um clique no botão
- Reseta com um clique longo
- Formato M:SS — fácil de ler de longe

### Indicadores visuais
- Fita de 8 LEDs coloridos que indicam o estado do dispositivo:
  - **Branco** ao ligar
  - **Verde piscando** enquanto conecta ao WiFi
  - **Verde fixo** ao conectar, apagando suavemente
  - **Amarelo respirando** quando está no modo de configuração
  - **Azul em barra** mostrando o progresso de atualizações
  - **Vermelho** em caso de erro
- Feedback visual imediato ao pressionar os botões

### Conectividade WiFi
- Conecta à sua rede doméstica
- Se não houver configuração, cria uma rede própria para setup
- Acessível pelo navegador de qualquer dispositivo na mesma rede

### Interface web
- Acesse pelo IP do dispositivo no browser
- Veja o peso e o timer em tempo real
- Tare diretamente pela interface
- Sem necessidade de aplicativo

### Configuração pelo browser
- Selecione e configure a rede WiFi sem precisar de cabo
- Reconfigure ou apague as credenciais quando quiser
- Ajuste o brilho dos LEDs com preview ao vivo
- Calibre a balança informando um peso conhecido
- Atualize o firmware remotamente (OTA) sem tirar o dispositivo do lugar

---

## Botões

| Botão | Clique | Clique longo |
|---|---|---|
| Botão 1 | Inicia / pausa o timer | Zera o timer |
| Botão 2 | Tara (zera o peso) | — |

---

## Display

O display OLED mostra simultaneamente o timer e o peso, com fontes grandes pensadas para leitura rápida durante o preparo, sem precisar se aproximar.

---

## Calibração

A balança precisa ser calibrada uma única vez com um peso conhecido (por exemplo, 1kg). Após isso, os dados ficam salvos e o dispositivo lembra a calibração mesmo após desligar. A calibração pode ser refeita a qualquer momento pela interface web.

---

## Ligações

![Diagrama de ligações](docs/schematic.svg)

### ESP32-C3 Super Mini — mapa de pinos

```
                    ┌─────────────────┐
                    │  ESP32-C3 Super │
                    │      Mini       │
              3.3V ─┤ 3V3         GND ├─ GND
               GND ─┤ GND         5V  ├─ 5V
     [Buzzer +] ───┤ GPIO2      GPIO10├─── [Botão 2] ── GND
                    │  GPIO3     GPIO9 ├─── [Botão 1] ── GND
                    │  GPIO4     GPIO8 ├─── (LED embutido)
     [WS2812B] ───┤ GPIO5      GPIO7 ├─── SCL (OLED + NAU7802)
                    │  GPIO6     GPIO6 ├─── SDA (OLED + NAU7802)
                    └─────────────────┘
```

### Componentes e conexões detalhadas

#### Display OLED — SSD1306 128×64 (I2C)
```
OLED          ESP32-C3
────          ────────
VCC  ───────  3.3V
GND  ───────  GND
SDA  ───────  GPIO 6
SCL  ───────  GPIO 7
```

#### Sensor de peso — NAU7802 (I2C)
```
NAU7802       ESP32-C3
───────       ────────
VDD  ───────  3.3V
GND  ───────  GND
SDA  ───────  GPIO 6   (compartilha com OLED)
SCL  ───────  GPIO 7   (compartilha com OLED)

NAU7802       Célula de carga
───────       ──────────────
E+   ───────  Excitação +
E-   ───────  Excitação -
A+   ───────  Sinal +
A-   ───────  Sinal -
```

#### Fita de LEDs — WS2812B (8 LEDs)
```
WS2812B       ESP32-C3
───────       ────────
VCC  ───────  5V
GND  ───────  GND
DIN  ───────  GPIO 5
```
> Recomendado: resistor de 300–500Ω em série no DIN e capacitor de 100µF entre VCC e GND.

#### Botões (2×)
```
Botão 1:  GPIO 9  ── Botão ── GND   (pull-up interno ativo)
Botão 2:  GPIO 10 ── Botão ── GND   (pull-up interno ativo)
```

#### Buzzer passivo
```
Buzzer        ESP32-C3
──────        ────────
+    ───────  GPIO 2
-    ───────  GND
```
> Para buzzer passivo de 5V, adicionar transistor NPN (ex: 2N2222) entre GPIO2 e o buzzer.

### Resumo de pinos

| GPIO | Função | Componente |
|------|--------|------------|
| 2 | PWM (buzzer) | Buzzer passivo |
| 5 | Data | Fita WS2812B |
| 6 | I2C SDA | OLED + NAU7802 |
| 7 | I2C SCL | OLED + NAU7802 |
| 9 | Botão 1 (pull-up) | Botão — Timer |
| 10 | Botão 2 (pull-up) | Botão — Tara |
