#include <stdio.h>
#include "pico/stdlib.h"
#include "utils/send_data.h"
#include "utils/wifi_connect.h"

#define BUTTON_A 5          // Definição do pino para o botão A.
#define BUTTON_B 6          // Definição do pino para o botão B.
#define LED_PIN_GREEN 11    // Definição do pino para o LED verde.
#define LED_PIN_BLUE 12     // Definição do pino para o LED azul.

volatile bool button_a_pressed = false; // Variável para armazenar o estado do botão A
volatile bool button_b_pressed = false; // Variável para armazenar o estado do botão B

void setup() {
    stdio_init_all(); // Inicializa a comunicação serial
    // init_wifi(); // Inicializa o Wi-Fi e conecta na rede

    gpio_init(LED_PIN_BLUE);                // Inicializa os pinos do LED azul.
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);   // Configura o pino do LED azul como saída.
    gpio_init(LED_PIN_GREEN);               // Inicializa os pinos do LED verde.
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);  // Configura o pino do LED verde como saída.
    gpio_init(BUTTON_A);                    // Inicializa o pino do botão A.
    gpio_set_dir(BUTTON_A, GPIO_IN);        // Configura o pino do botão A como entrada.
    gpio_pull_up(BUTTON_A);                 // Ativa o resistor pull-up interno para o botão A.
    gpio_init(BUTTON_B);                    // Inicializa o pino do botão B.
    gpio_set_dir(BUTTON_B, GPIO_IN);        // Configura o pino do botão B como entrada.
    gpio_pull_up(BUTTON_B);                 // Ativa o resistor pull-up interno para o botão B.
}

void button_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A) {
        if (events & GPIO_IRQ_EDGE_FALL) {
            printf("Botão A pressionado\n");
            button_a_pressed = true;
        } else if (events & GPIO_IRQ_EDGE_RISE) {
            printf("Botão A liberado\n");
            button_a_pressed = false;
        }
    } else if (gpio == BUTTON_B) {
        if (events & GPIO_IRQ_EDGE_FALL) {
            printf("Botão B pressionado\n");
            button_b_pressed = true;
        } else if (events & GPIO_IRQ_EDGE_RISE) {
            printf("Botão B liberado\n");
            button_b_pressed = false;
        }
    }
}

int main() {
    setup(); // Chama a função de configuração

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);

    sleep_ms(3000); // Espera 3 segundo para estabilizar o sistema
    printf("Sistema iniciado\n");

    int data = 0;

    while (true) {
        // printf("Enviando dados para o servidor...\n");
        // create_request(data++); // Envia para o servidor (0, 1, 2, 3...)
        // sleep_ms(3000); // Espera 3 segundos antes do próximo envio

        if (button_a_pressed) {
            gpio_put(LED_PIN_GREEN, 1);
        } else {
            gpio_put(LED_PIN_GREEN, 0);
        }

        if (button_b_pressed) {
            gpio_put(LED_PIN_BLUE, 1);
        } else {
            gpio_put(LED_PIN_BLUE, 0);
        }

        tight_loop_contents();
    }
}