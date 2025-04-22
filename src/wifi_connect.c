#include "wifi_connect.h"

bool is_wifi_connected() {
    return cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_JOIN;
}

// Inicialização do wifi e conectando na rede
void init_wifi() {
    // Inicialização do Wi-Fi
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar Wi-Fi\n");
        return;
    }

    // Ativação do modo STA (cliente)
    cyw43_arch_enable_sta_mode();

    // Conectando ao Wi-Fi com timeout de 10 segundos
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Erro ao conectar ao Wi-Fi: tentando novamente...\n");
        sleep_ms(3000);
    }

    // while (true) {
    //     if (!is_wifi_connected()) {
    //         printf("Wi-Fi desconectado, tentando reconectar...\n");
    //         cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);
    //         sleep_ms(5000);
    //     } else {
    //         sleep_ms(1000);
    //     }
    // }

    printf("Wi-Fi conectado com sucesso!\n");
}