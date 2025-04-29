#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "temperature.h"

void temperature_init() {
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4); // Canal do sensor de temperatura interno
}

float read_internal_temperature() {
    const float conversion_factor = 3.3f / (1 << 12);
    adc_select_input(4); // Canal do sensor de temperatura interno
    uint16_t raw = adc_read();
    float voltage = raw * conversion_factor;
    
    // Fórmula para conversão baseada no datasheet do RP2040
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
    
    return temperature;
}