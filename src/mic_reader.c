#include <math.h>
#include <stdlib.h>
#include "mic_reader.h"

/**  Função que inicializa o leitor de microfone (configura ADC, DMA e buffer) */
void mic_reader_init(MicReader *reader, uint mic_channel, uint sample_count, float adc_clock_div) {
    // Salva configurações iniciais
    reader->mic_channel = mic_channel;
    reader->mic_pin = 26 + mic_channel; // Pino correspondente ao canal do ADC
    reader->sample_count = sample_count;
    reader->adc_clock_div = adc_clock_div;
    
    // Aloca espaço para guardar as amostras
    reader->adc_buffer = (uint16_t *)malloc(sample_count * sizeof(uint16_t));
    
    // Inicializa o pino ADC e configura para o canal desejado
    adc_gpio_init(reader->mic_pin);
    adc_init();
    adc_select_input(reader->mic_channel);

    // Configura o FIFO do ADC (buffer interno do ADC)
    adc_fifo_setup(
        true,    // Habilita o FIFO
        true,    // Habilita para DMA ler os dados automaticamente
        1,       // Solicita leitura a cada nova amostra
        false,   // Sem uso de bits de erro
        false    // Mantém amostras com 12 bits
    );

    // Define a frequência de amostragem via divisor de clock
    adc_set_clkdiv(reader->adc_clock_div);

    // Configuração do canal DMA para ler do ADC
    reader->dma_channel = dma_claim_unused_channel(true);
    reader->dma_cfg = dma_channel_get_default_config(reader->dma_channel);

    // Configura como o DMA transfere os dados
    channel_config_set_transfer_data_size(&reader->dma_cfg, DMA_SIZE_16); // 16 bits
    channel_config_set_read_increment(&reader->dma_cfg, false); // Endereço fixo (FIFO)
    channel_config_set_write_increment(&reader->dma_cfg, true); // Vai gravando sequencialmente no buffer
    channel_config_set_dreq(&reader->dma_cfg, DREQ_ADC); // Lê os dados sempre que ADC tiver novo valor
}

/**  Função que desliga e libera recursos do leitor de microfone */
void mic_reader_deinit(MicReader *reader) {
    free(reader->adc_buffer); // Libera a memória do buffer
    dma_channel_unclaim(reader->dma_channel); // Libera o canal DMA usado
}

/**  Função que realiza a leitura das amostras do microfone e salva no buffer */
void mic_reader_sample(MicReader *reader) {
    adc_fifo_drain(); // Limpa qualquer amostra antiga no FIFO
    adc_run(false);   // Garante que ADC está parado antes de configurar

    // Configura o DMA para copiar as amostras do FIFO para o buffer
    dma_channel_configure(reader->dma_channel, &reader->dma_cfg,
        reader->adc_buffer,  // Para onde vai copiar
        &(adc_hw->fifo),     // De onde vai copiar
        reader->sample_count, // Quantas amostras
        true                 // Inicia a transferência agora
    );

    adc_run(true); // Inicia o ADC
    dma_channel_wait_for_finish_blocking(reader->dma_channel); // Espera até terminar
    adc_run(false); // Para o ADC após capturar tudo
}

/**  Função que calcula o nível de volume (dB) com base nas amostras capturadas */
float mic_reader_calculate_db(MicReader *reader) {
    float sum_squares = 0.0f;

    // Calcula o valor RMS (raiz da média dos quadrados das amostras)
    for (uint i = 0; i < reader->sample_count; ++i) {
        // Converte a amostra de ADC para voltagem e centraliza em 0V (remove offset de 1.65V)
        float sample = (reader->adc_buffer[i] * 3.3f / (1 << 12u)) - 1.65f;
        sum_squares += sample * sample;
    }

    float rms = sqrtf(sum_squares / reader->sample_count);

    if (rms <= 0.0000001f) return -100.0f; // Se quase zero, retorna valor mínimo de dB

    // Converte RMS para decibéis (dB) relativo ao limiar de audição (20uPa)
    return 20.0f * log10f(rms / 0.00002f);
}

/**  Função que traduz o valor em dB para uma intensidade simplificada (de 0 a 4) */
uint8_t mic_reader_calculate_intensity(float db) {
    // Faixas de dB definidas para classificar intensidade
    const float db_ranges[] = {50.0f, 60.0f, 70.0f, 80.0f};
    uint8_t intensity = 0;
    
    // Percorre as faixas para determinar a intensidade
    for (; intensity < 4; intensity++) {
        if (db < db_ranges[intensity]) {
            break;
        }
    }
    
    return intensity; // Retorna um valor de 0 (baixo) até 4 (alto)
}