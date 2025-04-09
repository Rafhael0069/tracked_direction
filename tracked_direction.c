#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
// #include "utils/send_data.h"
// #include "utils/wifi_connect.h"
#include "utils/joystick.h"
#include "utils/matrix_control.h"
#include "utils/matrices.h"

#define BUTTON_A 5          // Definição do pino para o botão A.
#define BUTTON_B 6          // Definição do pino para o botão B.
#define LED_PIN_GREEN 11    // Definição do pino para o LED verde.
#define LED_PIN_BLUE 12     // Definição do pino para o LED azul.

volatile bool button_a_pressed = false; // Variável para armazenar o estado do botão A
volatile bool button_b_pressed = false; // Variável para armazenar o estado do botão B

const int brightness = 160;         // Brilho médio para os LEDs da matriz.
// const int brightness = 200;         // Brilho alto para os LEDs da matriz.

/** Inicializador do pinos e leds */
void init_gpio(){
    gpio_init(LED_PIN_GREEN);                // Inicializa os pinos do LED verde.
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);   // Configura o pino do LED verde como saída.
    gpio_init(LED_PIN_BLUE);                 // Inicializa os pinos do LED azul.
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);    // Configura o pino do LED azul como saída.
    gpio_init(BUTTON_A);                     // Inicializa o pino do botão A.
    gpio_set_dir(BUTTON_A, GPIO_IN);         // Configura o pino do botão A como entrada.
    gpio_pull_up(BUTTON_A);                  // Ativa o resistor pull-up interno para o botão A.
    gpio_init(BUTTON_B);                     // Inicializa o pino do botão B.
    gpio_set_dir(BUTTON_B, GPIO_IN);         // Configura o pino do botão B como entrada.
    gpio_pull_up(BUTTON_B);                  // Ativa o resistor pull-up interno para o botão B.
}

/** Inicializador geral */
void setup() {
    stdio_init_all();          // Inicializa a comunicação serial
    joystick_init();           // Inicializa o joystick
    // init_wifi();               // Inicializa o Wi-Fi e conecta na rede
    npInit(LED_PIN);           // Inicializa a matriz de LEDs.
    setBrightness(brightness); // Define o brilho da matriz de LEDs.
    init_gpio();               // Inicializa os pinos GPIO.
}

/** Callback para interrupções dos botões */
void button_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A) {
        button_a_pressed = (events & GPIO_IRQ_EDGE_FALL);
        printf("Botão A %s\n", button_a_pressed ? "pressionado" : "liberado");
    } else if (gpio == BUTTON_B) {
        button_b_pressed = (events & GPIO_IRQ_EDGE_FALL);
        printf("Botão B %s\n", button_b_pressed ? "pressionado" : "liberado");
    }
}

/** Função para retornar o valor do eixo x */
const char* get_cardinal_x(int16_t value) {
    if (value > 1) {
        updateMatrix(north_arrow, 0, 255, 0); // Atualiza a matriz com o padrão de seta para o norte
        return "Norte";
    } else if (value < -1) {
        updateMatrix(south_arrow, 0, 255, 0); // Atualiza a matriz com o padrão de seta para o sul
        return "Sul";
    } else {
        return NULL; // Retorna NULL se o valor for 0
    }
}

/** Função para retornar o valor do eixo y */
const char* get_cardinal_y(int16_t value) {
    if(value < -1) {
        updateMatrix(west_arrow, 0, 255, 0); // Atualiza a matriz com o padrão de seta para o oeste
        return "Oeste";
    } else if (value > 1) {
        updateMatrix(east_arrow, 0, 255, 0); // Atualiza a matriz com o padrão de seta para o leste
        return "Leste";
    } else {
        return NULL; // Retorna NULL se o valor for 0
    }
}

/** Função para retornar a direção da bússola com base nos valores x e y */
const char* get_compass_direction(int16_t x, int16_t y) {

    if (x > 1 && y > 0) {
        updateMatrix(northwest_arrow, 0, 255, 0);
        return "Noroeste";
    } else if (x > 1 && y <= 0) {
        updateMatrix(northeast_arrow, 0, 255, 0);
        return "Nordeste";
    } else if (x < -1 && y > 0) {
        updateMatrix(southwest_arrow, 0, 255, 0);
        return "Sudoeste";
    } else if (x < -1 && y <= 0) {
        updateMatrix(southeast_arrow, 0, 255, 0);
        return "Sudeste";
    }
    return NULL;
}

/** Função para processar o estado do joystick e atualizar a matriz de LEDs */
void process_joystick(JoystickState current, JoystickState* last) {
    if (current.x != last->x || current.y != last->y) {
        printf("Posição: X=%+3d, Y=%+3d", current.x, current.y);

        if (current.x >= -1 && current.x <= 1 && current.y >= -1 && current.y <= 1) {
            updateMatrix(center_pattern, 0, 255, 0);
            printf(" (Centro)");
        } else {
            int16_t abs_x = abs(current.x);
            int16_t abs_y = abs(current.y);
            int16_t diff = abs(abs_x - abs_y);
            bool show_compass = (abs_x > 0 && abs_y > 0 && diff <= 4);

            if (show_compass) {
                const char* compass = get_compass_direction(current.x, current.y);
                const char* primary = (abs_x > abs_y) ? get_cardinal_x(current.x) : get_cardinal_y(current.y);
                const char* secondary = (abs_x > abs_y) ? get_cardinal_y(current.y) : get_cardinal_x(current.x);
                printf(" %s (%s - %s)", compass, primary, secondary);
            } else {
                const char* main_dir = (abs_x > abs_y) ? get_cardinal_x(current.x) : get_cardinal_y(current.y);
                printf(" %s", main_dir);
            }
        }

        printf("\n");
        *last = current;
    }

    if (current.button_pressed) {
        printf("Botão pressionado!\n");
        sleep_ms(200); // Debounce
    }
}

/** Função para atualizar os LEDs com base no estado dos botões */
void update_leds() {
    gpio_put(LED_PIN_GREEN, button_a_pressed);
    gpio_put(LED_PIN_BLUE, button_b_pressed);
}

/** Função principal */
int main() {
    setup(); // Chama a função de configuração

    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);

    sleep_ms(3000); // Espera 3 segundo para estabilizar o sistema
    printf("Sistema iniciado\n");

    JoystickState current_state, last_state = {0};

    int data = 0;

    while (true) {
        // printf("Enviando dados para o servidor...\n");
        // create_request(data++); // Envia para o servidor (0, 1, 2, 3...)
        // sleep_ms(3000); // Espera 3 segundos antes do próximo envio

        current_state = joystick_read(); // Lê o estado atual do joystick
        update_leds();
        process_joystick(current_state, &last_state);
        tight_loop_contents();
    }
}