#ifndef MIC_READER_H
#define MIC_READER_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

// Configurações padrão
#define DEFAULT_MIC_CHANNEL 2
#define DEFAULT_MIC_PIN (26 + DEFAULT_MIC_CHANNEL)
#define DEFAULT_SAMPLE_COUNT 200
#define DEFAULT_ADC_CLOCK_DIV 96.f

// Estrutura para configuração do leitor de microfone
typedef struct {
    uint mic_channel;
    uint mic_pin;
    uint sample_count;
    float adc_clock_div;
    
    // Estado interno
    uint dma_channel;
    dma_channel_config dma_cfg;
    uint16_t *adc_buffer;
} MicReader;

// Inicializa o leitor de microfone
void mic_reader_init(MicReader *reader, uint mic_channel, uint sample_count, float adc_clock_div);

// Libera recursos do leitor de microfone
void mic_reader_deinit(MicReader *reader);

// Realiza uma leitura do microfone
void mic_reader_sample(MicReader *reader);

// Calcula o nível de pressão sonora em dB (valor relativo)
float mic_reader_calculate_db(MicReader *reader);

// Calcula a intensidade do som (0-4)
uint8_t mic_reader_calculate_intensity(MicReader *reader, float db);

#endif // MIC_READER_H