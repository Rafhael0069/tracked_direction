#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <stdint.h>
#include "hardware/pio.h"

#define LED_COUNT 25  // Número total de LEDs na matriz.
#define LED_PIN 7     // Pino GPIO onde os LEDs estão conectados.

#define MATRIX_SIZE 5  // Tamanho da matriz (5x5).

// Estrutura para representar um pixel (LED).
struct pixel_t {
  uint8_t G, R, B;  // Componentes de cor: verde, vermelho e azul.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;  // Tipo de dado para um LED.

// Funções disponíveis para controle da matriz.
void npInit(uint pin);
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void npClear();
void npWrite();
void setBrightness(uint8_t brightness);
void updateMatrix(int matrix[MATRIX_SIZE][MATRIX_SIZE], uint8_t r, uint8_t g, uint8_t b);

#endif // LED_MATRIX_H