#include "joystick.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

void joystick_init() {
    // Inicializa ADC
    adc_init();
    
    // Configura pinos do joystick
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    
    // Configura botão
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);
}

JoystickState joystick_read() {
    JoystickState state;
    uint16_t raw_x, raw_y;
    
    // Lê eixo X (normaliza para -10 a 10)
    adc_select_input(ADC_CHANNEL_X);
    sleep_us(2);
    raw_x = adc_read();
    state.x = (int16_t)((raw_x / 204.8) - 10); // 4096/20 = 204.8
    
    // Lê eixo Y (normaliza para -10 a 10)
    adc_select_input(ADC_CHANNEL_Y);
    sleep_us(2);
    raw_y = adc_read();
    state.y = (int16_t)((raw_y / 204.8) - 10);
    
    // Lê botão
    state.button_pressed = !gpio_get(SW_PIN);
    
    return state;
}