#include "pico/stdlib.h"      // Biblioteca padrão do Raspberry Pi Pico
#include "hardware/adc.h"     // Biblioteca para usar o conversor analógico-digital (ADC)
#include "temperature.h"     // Header deste módulo (provavelmente contém a declaração das funções)

/** Função para inicializar o sensor de temperatura interno */
void temperature_init() {
    adc_init();                         // Inicializa o módulo ADC do RP2040
    adc_set_temp_sensor_enabled(true);  // Habilita o sensor de temperatura interno
    adc_select_input(4);                // Seleciona o canal 4 do ADC (que é o canal do sensor interno)
}

/** Função para ler e retornar a temperatura interna do chip em graus Celsius */
float read_internal_temperature() {
    const float conversion_factor = 3.3f / (1 << 12); // Fator para converter valor bruto (0-4095) em tensão (0-3.3V)

    adc_select_input(4);   // Seleciona novamente o canal 4 (sensor interno)
    uint16_t raw = adc_read();  // Lê o valor bruto do ADC (0-4095)
    
    float voltage = raw * conversion_factor;  // Converte o valor lido em tensão (em volts)
    
    // Fórmula oficial do datasheet do RP2040 para converter tensão em temperatura
    float temperature = 27.0f - (voltage - 0.706f) / 0.001721f;
    
    return temperature;  // Retorna a temperatura calculada em graus Celsius
}