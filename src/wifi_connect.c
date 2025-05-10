#include "wifi_connect.h"
#include "wifi_credentials.h" // Arquivo de credenciais do Wi-Fi. Não deve ser versionado no Git.

#define LED_PIN_GREEN 11
#define LED_PIN_RED 13

// Inicialização do wifi e conectando na rede
void init_wifi() {

    // Inicialização do LED verde
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);

    // Inicialização do Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        gpio_put(LED_PIN_RED, 1); // Liga o LED vermelho
        sleep_ms(50);             // Espera 0.05 segundo
        gpio_put(LED_PIN_RED, 0); // Desliga o LED vermelho
        return;
    }

    // Ativação do modo STA (cliente)
    cyw43_arch_enable_sta_mode();

    // Conectando ao Wi-Fi com timeout de 10 segundos
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro ao conectar ao Wi-Fi %s : tentando novamente...\n", WIFI_SSID);
        gpio_put(LED_PIN_RED, 1); // Liga o LED vermelho
        sleep_ms(50);             // Espera 0.05 segundo
        gpio_put(LED_PIN_RED, 0); // Desliga o LED vermelho
        sleep_ms(200);            // Espera 0.2 segundo
        gpio_put(LED_PIN_RED, 1); // Liga o LED vermelho
        sleep_ms(50);             // Espera 0.05 segundo
        gpio_put(LED_PIN_RED, 0); // Desliga o LED vermelho
        sleep_ms(2700);
    }

    printf("Wi-Fi conectado com sucesso!\n");

    gpio_put(LED_PIN_GREEN, 1); // Liga o LED verde
    sleep_ms(50);               // Espera 0.05 segundo
    gpio_put(LED_PIN_GREEN, 0); // Desliga o LED verde
    sleep_ms(200);              // Espera 0.2 segundo
    gpio_put(LED_PIN_GREEN, 1); // Liga o LED verde
    sleep_ms(50);               // Espera 0.05 segundo
    gpio_put(LED_PIN_GREEN, 0); // Desliga o LED verde
}