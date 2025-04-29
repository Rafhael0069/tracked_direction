#include <stdio.h>
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
#include "temperature.h"
#include "utils/matrices.h"

// Definições de hardware
#define BUTTON_A 5
#define BUTTON_B 6
#define LED_PIN_GREEN 11
#define LED_PIN_BLUE 12
#define BRIGHTNESS 160

// Estrutura de dados do dispositivo
typedef struct {
    char direction[32];
    bool button_a;
    bool button_b;
    float db_level;
    uint8_t sound_intensity;
    float temperature;
} DeviceState;

// Variáveis globais
volatile bool button_a_pressed = false;
volatile bool button_b_pressed = false;
QueueHandle_t log_queue;
QueueHandle_t state_queue;

// Protótipos de funções
void init_gpio();
void setup();
void button_callback(uint gpio, uint32_t events);
const char* determine_direction(int16_t x, int16_t y);
void update_led_matrix(const char* direction);
void task_joystick(void *params);
void task_leds(void *params);
void task_logger(void *params);
void task_sensor_reader(void *params);
void task_data_sender(void *params);

/** Função de inicialização de botões */
void init_gpio() {
    gpio_init(LED_PIN_GREEN); gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_init(LED_PIN_BLUE);  gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_init(BUTTON_A);      gpio_set_dir(BUTTON_A, GPIO_IN);      gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B);      gpio_set_dir(BUTTON_B, GPIO_IN);      gpio_pull_up(BUTTON_B);
}

/** Função de inicialização do sistema */
void setup() {
    stdio_init_all();
    sleep_ms(5000); // Espera para estabilização
    joystick_init();
    init_wifi();
    npInit(LED_PIN);
    setBrightness(BRIGHTNESS);
    init_gpio();
    temperature_init(); 
}

/** Função de callback para os botões */
void button_callback(uint gpio, uint32_t events) {
    if (gpio == BUTTON_A) button_a_pressed = (events & GPIO_IRQ_EDGE_FALL);
    else if (gpio == BUTTON_B) button_b_pressed = (events & GPIO_IRQ_EDGE_FALL);
}

/** Função para determinar a direção com base nos valores do joystick */
const char* determine_direction(int16_t x, int16_t y) {
    if (x >= -1 && x <= 1 && y >= -1 && y <= 1) {
        updateMatrix(center_pattern, 0, 255, 0);
        return "Centro";
    }

    int16_t abs_x = abs(x), abs_y = abs(y);
    int16_t diff = abs(abs_x - abs_y);

    if (diff <= 4 && abs_x > 2 && abs_y > 2) {
        if (x >  0 && y >  0) { updateMatrix(northwest_arrow, 0, 255, 0); return "Noroeste"; }
        if (x >  0 && y <  0) { updateMatrix(northeast_arrow, 0, 255, 0); return "Nordeste"; }
        if (x <  0 && y >  0) { updateMatrix(southwest_arrow, 0, 255, 0); return "Sudoeste"; }
        if (x <  0 && y <  0) { updateMatrix(southeast_arrow, 0, 255, 0); return "Sudeste";  }
    }
    if (abs_x >= abs_y) {
        if (x > 2)  { updateMatrix(north_arrow, 0, 255, 0); return "Norte"; }
        if (x < -2) { updateMatrix(south_arrow, 0, 255, 0); return "Sul";   }
    } else {
        if (y < -2) { updateMatrix(west_arrow,  0, 255, 0); return "Oeste"; }
        if (y >  2) { updateMatrix(east_arrow,  0, 255, 0); return "Leste"; }
    }
    
    return "Desconhecido";
}

/** Tarefa: Leitura do joystick */
void task_joystick(void *params) {
    JoystickState current, last = {0};
    char log_msg[128];

    while (1) {
        current = joystick_read();

        if (current.x != last.x || current.y != last.y) {
            const char* dir = determine_direction(current.x, current.y);
            snprintf(log_msg, sizeof(log_msg), "Posição: X=%+3d, Y=%+3d %s", current.x, current.y, dir);
            xQueueSend(log_queue, &log_msg, portMAX_DELAY);
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
            printf("[LOG] %s\n", msg);
        }
    }
}

/** Tarefa: Coleta de dados para envio para servidor */
void task_sensor_reader(void *params) {
    DeviceState state;
    MicReader mic;
    mic_reader_init(&mic, DEFAULT_MIC_CHANNEL, DEFAULT_SAMPLE_COUNT, DEFAULT_ADC_CLOCK_DIV);

    while (1) {
        // Leitura do joystick
        JoystickState js = joystick_read();
        strncpy(state.direction, determine_direction(js.x, js.y), sizeof(state.direction));
        state.button_a = !gpio_get(BUTTON_A);
        state.button_b = !gpio_get(BUTTON_B);

        // Leitura do microfone
        mic_reader_sample(&mic);
        state.db_level = mic_reader_calculate_db(&mic);
        state.sound_intensity = mic_reader_calculate_intensity(state.db_level);

        // Leitura da temperatura
        state.temperature = read_internal_temperature();

        xQueueOverwrite(state_queue, &state);

        // Tempo quando inativo: 2 minutos e 15 segundos
        // Tempo mexendo no JoyStick: 3 minutos e 14 segundos
        // vTaskDelay(pdMS_TO_TICKS(1000)); // Tempo de espera entre leituras e envios (Requisitado)

        // Tempo quando inativo: 6 minutos e 40 segundos
        // Tempo mexendo no JoyStick: 3 minutos e 14 segundos mexendo
        vTaskDelay(pdMS_TO_TICKS(2000)); 
        
        // Tempo quando inativo: 10 minutos
        // Tempo mexendo no JoyStick: 2 minutos
        // vTaskDelay(pdMS_TO_TICKS(3000));
    }
    
    mic_reader_deinit(&mic);
}

/** Tarefa: Envio de dados ao servidor */
void task_data_sender(void *params) {
    DeviceState state;
    char json_payload[300];

    while (1) {
        if (xQueueReceive(state_queue, &state, portMAX_DELAY)) {
            snprintf(json_payload, sizeof(json_payload),
                "Direção: %s | Botão A: %s | Botão B: %s | Nível dB: %.2f | Intensidade: %d | Temp: %.2f°C",
                state.direction, state.button_a ? "Pressionado" : "Solto",
                state.button_b ? "Pressionado" : "Solto", state.db_level, 
                state.sound_intensity, state.temperature);

            printf("[ SEND ] %s\n", json_payload);
            create_request(json_payload);
        }
    }
}

/** Função principal */
int main() {
    setup();
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &button_callback);

    log_queue = xQueueCreate(5, sizeof(char[128]));
    state_queue = xQueueCreate(1, sizeof(DeviceState));

    if (!log_queue || !state_queue) {
        printf("Erro ao criar filas!\n");
        return 1;
    }

    xTaskCreate(task_joystick, "Joystick", 1024, NULL, 2, NULL);
    xTaskCreate(task_sensor_reader, "SensorReader", 1024, NULL, 2, NULL);
    xTaskCreate(task_leds, "LEDs", 512, NULL, 1, NULL);
    xTaskCreate(task_logger, "Logger", 1024, NULL, 1, NULL);
    xTaskCreate(task_data_sender, "DataSender", 1024, NULL, 2, NULL);

    vTaskStartScheduler();
    while (true) {}
}