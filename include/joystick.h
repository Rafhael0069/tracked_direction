#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h"

// Configurações do joystick
#define VRX_PIN 26
#define VRY_PIN 27
#define SW_PIN 22
#define ADC_CHANNEL_X 0
#define ADC_CHANNEL_Y 1

// Estrutura para armazenar estado do joystick
typedef struct {
    int16_t x;
    int16_t y;
    bool button_pressed;
} JoystickState;

// Inicializa o joystick
void joystick_init();

// Lê o estado atual do joystick (valores normalizados de -10 a 10)
JoystickState joystick_read();

#endif