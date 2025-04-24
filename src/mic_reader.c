#include "mic_reader.h"
#include <math.h>
#include <stdlib.h>

void mic_reader_init(MicReader *reader, uint mic_channel, uint sample_count, float adc_clock_div) {
    // Configuração básica
    reader->mic_channel = mic_channel;
    reader->mic_pin = 26 + mic_channel;
    reader->sample_count = sample_count;
    reader->adc_clock_div = adc_clock_div;
    
    // Aloca buffer para amostras
    reader->adc_buffer = (uint16_t *)malloc(sample_count * sizeof(uint16_t));
    
    // Configuração do hardware
    adc_gpio_init(reader->mic_pin);
    adc_init();
    adc_select_input(reader->mic_channel);

    adc_fifo_setup(
        true,    // Habilitar FIFO
        true,    // Habilitar request de dados do DMA
        1,       // Threshold para ativar request DMA
        false,   // Não usar bit de erro
        false    // Manter amostras em 12-bits
    );

    adc_set_clkdiv(reader->adc_clock_div);

    // Configuração do DMA
    reader->dma_channel = dma_claim_unused_channel(true);
    reader->dma_cfg = dma_channel_get_default_config(reader->dma_channel);

    channel_config_set_transfer_data_size(&reader->dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&reader->dma_cfg, false);
    channel_config_set_write_increment(&reader->dma_cfg, true);
    channel_config_set_dreq(&reader->dma_cfg, DREQ_ADC);
}

void mic_reader_deinit(MicReader *reader) {
    // Libera recursos
    free(reader->adc_buffer);
    dma_channel_unclaim(reader->dma_channel);
}

void mic_reader_sample(MicReader *reader) {
    adc_fifo_drain();
    adc_run(false);

    dma_channel_configure(reader->dma_channel, &reader->dma_cfg,
        reader->adc_buffer,
        &(adc_hw->fifo),
        reader->sample_count,
        true
    );

    adc_run(true);
    dma_channel_wait_for_finish_blocking(reader->dma_channel);
    adc_run(false);
}

float mic_reader_calculate_db(MicReader *reader) {
    float sum_squares = 0.0f;
    
    for (uint i = 0; i < reader->sample_count; ++i) {
        float sample = (reader->adc_buffer[i] * 3.3f / (1 << 12u)) - 1.65f;
        sum_squares += sample * sample;
    }
    
    float rms = sqrtf(sum_squares / reader->sample_count);
    
    // Convertendo para dB (valor relativo, não absoluto)
    return 20.0f * log10f(rms / 0.00002f);  // 0.00002 é o limiar de audição em Pascal
}

uint8_t mic_reader_calculate_intensity(MicReader *reader, float db) {
    // Ajuste esses valores conforme necessário para sua aplicação
    const float db_ranges[] = {50.0f, 60.0f, 70.0f, 80.0f};
    uint8_t intensity = 0;
    
    for (; intensity < 4; intensity++) {
        if (db < db_ranges[intensity]) {
            break;
        }
    }
    
    return intensity;
}