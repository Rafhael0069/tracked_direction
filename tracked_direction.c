#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "send_data.h"
#include "wifi_connect.h"
#include "joystick.h"
#include "matrix_control.h"
#include "mic_reader.h"
#include "utils/matrices.h"

#define BUTTON_A 5
#define BUTTON_B 6
#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12

volatile bool button_a_pressed = false;
volatile bool button_b_pressed = false;

typedef struct {
    char direction[32];
    bool button_a;
    bool button_b;
} DeviceState;

QueueHandle_t log_queue;
QueueHandle_t send_queue;
QueueHandle_t state_queue;


const int brightness = 160;

/** Função de inicialização de botões */
void init_gpio() {
    gpio_init(LED_PIN_GREEN); gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_init(LED_PIN_BLUE);  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_init(BUTTON_A);      gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);      gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
}

/** Função de inicialização do sistema */
void setup() {
    stdio_init_all();
    sleep_ms(5000); // Espera 5 segundos para estabilizar o sistema
    joystick_init();
    init_wifi();
    npInit(LED_PIN);
    setBrightness(brightness);
    init_gpio();
}

/** Função de callback para os botões */
void button_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A) button_a_pressed = (events & GPIO_IRQ_EDGE_FALL);
    else if (gpio == BUTTON_B) button_b_pressed = (events & GPIO_IRQ_EDGE_FALL);
}

/** Função de recuperação do eixo x */
const char* get_cardinal_x(int16_t value) {
    if (value > 1) {
        updateMatrix(north_arrow, 0, 255, 0); return "Norte";
    } else if (value < -1) {
        updateMatrix(south_arrow, 0, 255, 0); return "Sul";
    }
    return NULL;
}

/** Função de recuperação do eixo y */
const char* get_cardinal_y(int16_t value) {
    if (value < -1) {
        updateMatrix(west_arrow, 0, 255, 0); return "Oeste";
    } else if (value > 1) {
        updateMatrix(east_arrow, 0, 255, 0); return "Leste";
    }
    return NULL;
}

/** Função de recuperação da direção da bússola */
const char* get_compass_direction(int16_t x, int16_t y) {
    if (x > 1 && y > 0) {
        updateMatrix(northwest_arrow, 0, 255, 0); return "Noroeste";
    } else if (x > 1 && y <= 0) {
        updateMatrix(northeast_arrow, 0, 255, 0); return "Nordeste";
    } else if (x < -1 && y > 0) {
        updateMatrix(southwest_arrow, 0, 255, 0); return "Sudoeste";
    } else if (x < -1 && y <= 0) {
        updateMatrix(southeast_arrow, 0, 255, 0); return "Sudeste";
    }
    return NULL;
}

/** Tarefa: Leitura do joystick */
void task_joystick(void *params) {
    JoystickState current, last = {0};
    char log_msg[128];

    while (1) {
        current = joystick_read();

        if (current.x != last.x || current.y != last.y) {
            snprintf(log_msg, sizeof(log_msg), "Posição: X=%+3d, Y=%+3d", current.x, current.y);

            if (current.x >= -1 && current.x <= 1 && current.y >= -1 && current.y <= 1) {
                updateMatrix(center_pattern, 0, 255, 0);
                strncat(log_msg, " (Centro)", sizeof(log_msg) - strlen(log_msg) - 1);
            } else {
                int16_t abs_x = abs(current.x);
                int16_t abs_y = abs(current.y);
                int16_t diff = abs(abs_x - abs_y);
                bool show_compass = (abs_x > 0 && abs_y > 0 && diff <= 4);

                if (show_compass) {
                    const char *compass = get_compass_direction(current.x, current.y);
                    const char *primary = (abs_x > abs_y) ? get_cardinal_x(current.x) : get_cardinal_y(current.y);
                    const char *secondary = (abs_x > abs_y) ? get_cardinal_y(current.y) : get_cardinal_x(current.x);
                    snprintf(log_msg + strlen(log_msg), sizeof(log_msg) - strlen(log_msg), " %s (%s - %s)", compass, primary, secondary);
                } else {
                    const char *main_dir = (abs_x > abs_y) ? get_cardinal_x(current.x) : get_cardinal_y(current.y);
                    snprintf(log_msg + strlen(log_msg), sizeof(log_msg) - strlen(log_msg), " %s", main_dir);
                }
            }

            xQueueSend(log_queue, &log_msg, portMAX_DELAY);
            xQueueSend(send_queue, &log_msg, portMAX_DELAY);
            last = current;
        }

        if (current.button_pressed) {
            const char* btn_msg = "Botão pressionado!";
            xQueueSend(log_queue, &btn_msg, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/** Tarefa: Atualização dos LEDs */
void task_leds(void *params) {
    while (1) {
        gpio_put(LED_PIN_GREEN, button_a_pressed);
        gpio_put(LED_PIN_BLUE, button_b_pressed);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/** Tarefa: Impressão dos logs */
void task_logger(void *params) {
    char msg[128];
    while (1) {
        if (xQueueReceive(log_queue, &msg, portMAX_DELAY)) {
            printf("[ LOG  ] %s\n", msg);
        }
    }
}

/** Tarefa: Coleta de dados para envio para servidor */
void task_data_collector(void *params) {
    DeviceState state;

    while (1) {
        JoystickState js = joystick_read();
        const char *direction = NULL;

        int16_t x = js.x;
        int16_t y = js.y;

        // Verifica se está no centro
        if (x >= -1 && x <= 1 && y >= -1 && y <= 1) {
            strncpy(state.direction, "Centro", sizeof(state.direction));
        } else {
            int16_t abs_x = abs(x);
            int16_t abs_y = abs(y);
            int16_t diff = abs(abs_x - abs_y);
            bool show_compass = (abs_x > 0 && abs_y > 0 && diff <= 4);

            if (show_compass) {
                const char *compass = get_compass_direction(x, y);
                strncpy(state.direction, compass, sizeof(state.direction));
            } else {
                const char *main_dir = (abs_x > abs_y) ? get_cardinal_x(x) : get_cardinal_y(y);
                strncpy(state.direction, main_dir, sizeof(state.direction));
            }
        }

        state.button_a = !gpio_get(BUTTON_A);
        state.button_b = !gpio_get(BUTTON_B);

        xQueueOverwrite(state_queue, &state);
        vTaskDelay(pdMS_TO_TICKS(1000));  // Tempo de espera padrão para coleta de dados
        // vTaskDelay(pdMS_TO_TICKS(4000));  // 4 segundos, só para teste
        // vTaskDelay(pdMS_TO_TICKS(5000));  // 5 segundos, só para teste

    }
}

/** Tarefa: Envio de dados ao servidor */
void task_sender(void *params) {
    DeviceState state;
    char msg[128];

    MicReader reader;
    mic_reader_init(&reader, DEFAULT_MIC_CHANNEL, DEFAULT_SAMPLE_COUNT, DEFAULT_ADC_CLOCK_DIV);

    while (1) {
        mic_reader_sample(&reader);   // Realiza uma leitura
        float db_level = mic_reader_calculate_db(&reader);  // Calcula o nível em dB
        uint8_t intensity = mic_reader_calculate_intensity(&reader, db_level); // Calcula a intensidade (0-4)

        if (xQueueReceive(state_queue, &state, portMAX_DELAY)) {
            snprintf(msg, sizeof(msg), "Direção: %s | Botão A: %s | Botão B: %s | Nível dB: %.2f | Intensidade: %d",
                     state.direction,
                     state.button_a ? "Pressionado" : "Solto",
                     state.button_b ? "Pressionado" : "Solto",
                     db_level,
                     intensity);

            printf("[ SEND ] Enviando para servidor: %s\n", msg);
            create_request(msg);
        }
    }

    mic_reader_deinit(&reader);
}

/** Função principal */
int main() {
    
    setup();
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);

    log_queue = xQueueCreate(5, sizeof(char[128]));
    send_queue = xQueueCreate(5, sizeof(char[128]));
    state_queue = xQueueCreate(1, sizeof(DeviceState));

    printf("Sistema iniciado\n");

    if (log_queue == NULL || send_queue == NULL || state_queue == NULL) {
        printf("Erro ao criar filas!\n");
        return 1;
    } else {
        printf("Filas criadas com sucesso!\n");
    }

    xTaskCreate(task_joystick, "Joystick", 1024, NULL, 2, NULL);
    xTaskCreate(task_data_collector, "DataCollector", 1024, NULL, 2, NULL);
    xTaskCreate(task_leds, "LEDs", 512, NULL, 1, NULL);
    xTaskCreate(task_logger, "Logger", 1024, NULL, 1, NULL);
    xTaskCreate(task_sender, "Sender", 1024, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true) {}
}