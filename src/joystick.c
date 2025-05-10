#include "joystick.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

/** Inicializa o joystick configurando os pinos do ADC e do botão. */
void joystick_init() {
    
    adc_init(); // Inicializa o periférico ADC para permitir leitura analógica.
    adc_gpio_init(VRX_PIN); // Configura o pino do eixo X do joystick para funcionar com ADC.
    adc_gpio_init(VRY_PIN); // Configura o pino do eixo Y do joystick para funcionar com ADC.
    gpio_init(SW_PIN);      // Inicializa o pino do botão do joystick.
    gpio_set_dir(SW_PIN, GPIO_IN); // Define o pino do botão como entrada.
    gpio_pull_up(SW_PIN);   // Ativa um resistor de pull-up no pino do botão.
}

/** Lê a posição atual do joystick e o estado do botão.
 *  Retorna uma estrutura JoystickState com valores normalizados. */
JoystickState joystick_read() {
    JoystickState state; // Estrutura para armazenar o estado atual do joystick.
    uint16_t raw_x, raw_y; // Variáveis para armazenar as leituras brutas do ADC.
    
    // Lê a posição do eixo X (converte de 0-4095 para -10 a 10).
    adc_select_input(ADC_CHANNEL_X); // Seleciona canal ADC do eixo X.
    sleep_us(2); // Espera 2 microsegundos para estabilizar a leitura.
    raw_x = adc_read(); // Lê valor bruto (0-4095).
    state.x = (int16_t)((raw_x / 204.8) - 10); // Normaliza para -10 a 10.
    
    // Lê a posição do eixo Y (converte de 0-4095 para -10 a 10).
    adc_select_input(ADC_CHANNEL_Y); // Seleciona canal ADC do eixo Y.
    sleep_us(2); // Espera 2 microsegundos.
    raw_y = adc_read(); // Lê valor bruto.
    state.y = (int16_t)((raw_y / 204.8) - 10); // Normaliza.
    
    // Lê o estado do botão (pressionado = 1, não pressionado = 0).
    state.button_pressed = !gpio_get(SW_PIN); // Inverte porque o botão está com pull-up.
    
    return state; // Retorna o estado completo do joystick.
}