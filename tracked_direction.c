#include <stdio.h>
#include "pico/stdlib.h"
#include "utils/send_data.h"
#include "utils/wifi_connect.h"


int main() {
    stdio_init_all();
    init_wifi();

    int data = 0;

    while (true) {
        printf("Enviando dados para o servidor...\n");

        create_request(data++); // Envia para o servidor (0, 1, 2, 3...)

        sleep_ms(3000); // Espera 3 segundos antes do próximo envio
    }
}