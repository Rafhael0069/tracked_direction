#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"
#include "matrix_control.h"

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;  // Instância do PIO (Programmable I/O) utilizada para controlar os LEDs.
uint sm;     // State Machine (máquina de estado) do PIO.

// Variáveis para controle de brilho com PWM
uint8_t current_brightness = 255;  // Brilho máximo por padrão (0-255)

/** Inicializa o PIO e configura a matriz de LEDs WS2818B no pino especificado. */
void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);  // Adiciona o programa WS2818B ao PIO.
  np_pio = pio0;  // Usa o PIO0.

  sm = pio_claim_unused_sm(np_pio, false);  // Tenta alocar uma máquina de estado livre no PIO0.
  if (sm == -1) {
    np_pio = pio1;  // Se não houver máquinas livres no PIO0, tenta no PIO1.
    sm = pio_claim_unused_sm(np_pio, true);  // Aloca uma máquina de estado no PIO1.
  }

  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);  // Inicializa o programa WS2818B no PIO.

  // Limpa o buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/** Ajusta o brilho global da matriz de LEDs. */
void setBrightness(uint8_t brightness) {
  current_brightness = brightness;  // Atualiza o valor de brilho global.
}

/** Aplica o brilho atual a uma cor RGB. */
uint8_t applyBrightness(uint8_t color) {
  return (color * current_brightness) / 255;  // Ajusta a cor com base no brilho.
}

/** Atribui uma cor RGB a um LED, aplicando o brilho. */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = applyBrightness(r);  // Aplica brilho ao componente vermelho.
  leds[index].G = applyBrightness(g);  // Aplica brilho ao componente verde.
  leds[index].B = applyBrightness(b);  // Aplica brilho ao componente azul.
}

/** Desliga todos os LEDs da matriz, limpando o buffer. */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    npSetLED(i, 0, 0, 0);  // Define todas as cores como 0 (LEDs desligados).
  }
}

/** Envia os dados do buffer para os LEDs físicos, atualizando as cores. */
void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);  // Envia o componente verde.
    pio_sm_put_blocking(np_pio, sm, leds[i].R);  // Envia o componente vermelho.
    pio_sm_put_blocking(np_pio, sm, leds[i].B);  // Envia o componente azul.
  }
  sleep_us(100);  // Espera 100us, sinal de RESET do datasheet do WS2818B.
}

/** Atualiza a matriz inteira com base em uma matriz 5x5 onde 1 acende e 0 apaga. */
void updateMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE], uint8_t r, uint8_t g, uint8_t b) {
  for (int y = 0; y < MATRIX_SIZE; y++) {
    for (int x = 0; x < MATRIX_SIZE; x++) {
      int index = y * MATRIX_SIZE + x;  // Calcula o índice no array linear.
      if (matrix[y][x]) {
        npSetLED(index, r, g, b);  // Ativa o LED com a cor especificada.
      } else {
        npSetLED(index, 0, 0, 0);  // Desativa o LED.
      }
    }
  }
  npWrite();  // Atualiza a matriz de LEDs.
}