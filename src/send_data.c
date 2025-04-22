#include "send_data.h"

#define SERVER_IP "192.168.1.79"        // ip do servidor
#define SERVER_PORT 3000                // porta do servidor
#define SERVER_PATH "/receber"          // rota de envio de dados
#define SERVER_STATUS_PATH "/status"    // rota de teste de status do servidor

#define TCP_TIMEOUT_MS 5000
#define MAX_RETRIES 3

// Callback para quando os dados são enviados
static err_t sent_callback(void *arg, struct tcp_pcb *pcb, u16_t len) {
    printf("[RESPONSE] Dados enviados com sucesso!\n");
    tcp_close(pcb); // Fecha a conexão TCP
    return ERR_OK;
}

// Modifique a função send_data_to_server
void send_data_to_server(const char *path, char *request_body, const char *type_method) {
    int retry_count = 0;
    err_t err;
    
    while (retry_count < MAX_RETRIES) {
        struct tcp_pcb *pcb = tcp_new();
        if (!pcb) {
            printf("[ ERRO ] Erro ao criar PCB (tentativa %d/%d)\n", retry_count+1, MAX_RETRIES);
            retry_count++;
            sleep_ms(1000);
            continue;
        }

        ip_addr_t server_ip;
        server_ip.addr = ipaddr_addr(SERVER_IP);

        // Configura timeout
        // Configure timeout using setsockopt or handle it externally
        // printf("[ INFO ] Timeout configuration is not directly supported on tcp_pcb.\n");
        
        err = tcp_connect(pcb, &server_ip, SERVER_PORT, NULL);
        if (err != ERR_OK) {
            printf("[ ERRO ] Erro ao conectar ao servidor (tentativa %d/%d)\n", retry_count+1, MAX_RETRIES);
            tcp_abort(pcb);
            retry_count++;
            sleep_ms(1000);
            continue;
        }

        char request[1024]; // Aumente o buffer
        int request_len = snprintf(request, sizeof(request),
                 "%s %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %d\r\n"
                 "Connection: close\r\n"
                 "\r\n"
                 "%s",
                 type_method, path, SERVER_IP, strlen(request_body), request_body);

        if (request_len >= sizeof(request)) {
            printf("[ ERRO ] Requisição muito grande para o buffer\n");
            tcp_abort(pcb);
            return;
        }

        tcp_sent(pcb, sent_callback);

        err = tcp_write(pcb, request, request_len, TCP_WRITE_FLAG_COPY);
        if (err != ERR_OK) {
            printf("[ ERRO ] Erro ao escrever dados (tentativa %d/%d)\n", retry_count+1, MAX_RETRIES);
            tcp_abort(pcb);
            retry_count++;
            sleep_ms(1000);
            continue;
        }

        err = tcp_output(pcb);
        if (err != ERR_OK) {
            printf("[ ERRO ] Erro ao enviar dados (tentativa %d/%d)\n", retry_count+1, MAX_RETRIES);
            tcp_abort(pcb);
            retry_count++;
            sleep_ms(1000);
            continue;
        }

        // Aguarde um pouco antes de fechar para garantir o envio
        sleep_ms(100);
        tcp_close(pcb);
        return;
    }

    printf("[ ERRO ] Falha após %d tentativas\n", MAX_RETRIES);
}

// Envia dados para o servidor
// void send_data_to_server(const char *path, char *request_body, const char *type_method) {
//     // Criando PCB
//     struct tcp_pcb *pcb = tcp_new();
//     if (!pcb) {
//         printf("[ ERRO ] Erro ao criar PCB\n");
//         return;
//     } else {
//         printf("[ INFO ] PCB criado com sucesso\n");
//     }

//     // Organizando o endereço IP
//     ip_addr_t server_ip;
//     server_ip.addr = ipaddr_addr(SERVER_IP);

//     // Conectando ao servidor
//     if (tcp_connect(pcb, &server_ip, SERVER_PORT, NULL) != ERR_OK) {
//         printf("[ ERRO ] Erro ao conectar ao servidor\n");
//         tcp_abort(pcb);
//         return;
//     } else {
//         printf("[ INFO ] Conectado ao servidor %s:%d\n", SERVER_IP, SERVER_PORT);
//     }

//     // Montando requisição
//     char request[521];
//     snprintf(request, sizeof(request),
//              "%s %s HTTP/1.1\r\n"
//              "Host: %s\r\n"
//              "Content-Type: application/json\r\n"
//              "Content-Length: %d\r\n"
//              "\r\n"
//              "%s",
//              type_method, path, SERVER_IP, strlen(request_body), request_body);

//     // Definindo o callback para quando os dados forem enviados com sucesso
//     tcp_sent(pcb, sent_callback);

//     // Empacotando a requisição
//     if (tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY) != ERR_OK) {
//         printf("[ ERRO ] Erro ao enviar dados\n");
//         tcp_abort(pcb);
//         return;
//     } else {
//         printf("[ INFO ] Dados enviados: %s\n", request);
//     }

//     // Enviando a requisição
//     if (tcp_output(pcb) != ERR_OK) {
//         printf("[ ERRO ] Erro ao enviar dados (tcp_output)\n");
//         tcp_abort(pcb);
//         return;
//     } else {
//         printf("[ INFO ] Dados enviados com sucesso (tcp_output)\n");
//     }
// }

// Criando uma requisição
void create_request(char *data) {
    const char *type_method = "POST";
    const char *path = SERVER_PATH;
    char json_request[256];

    // Preparando o corpo da requisição
    snprintf(json_request, sizeof(json_request),
             "{ \"dado\" : \"%s\" }",
             data);

    // Enviando requisição para o servidor
    send_data_to_server(path, json_request, type_method);
}





/** VERIFICAÇÃO DE CONECÇÃO COM O SERVIDOR */
// Callback quando receber resposta do servidor
static err_t recv_callback(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        char response[512];
        memset(response, 0, sizeof(response));
        memcpy(response, p->payload, p->len);
        printf("[ CHECK SERVER ] Resposta do servidor (/status): \n%s\n", response);

        tcp_recved(pcb, p->len);
        pbuf_free(p);
        tcp_close(pcb);
    } else {
        printf("[ CHECK SERVER ] Conexão encerrada pelo servidor.\n");
        tcp_close(pcb);
    }
    return ERR_OK;
}

// Callback de envio
static err_t sent_callback_status(void *arg, struct tcp_pcb *pcb, u16_t len) {
    printf("[ CHECK SERVER ] Requisição GET enviada com sucesso!\n");
    return ERR_OK;
}

// Chama o servidor e imprime resposta no monitor
void check_server_status() {
    // printf("[ CHECK SERVER ] [ 01 ]\n");
    printf("[ CHECK SERVER ] Verificando status do servidor...\n");
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("[ CHECK SERVER ] Erro ao criar PCB para /status\n");
        return;
    } else {
        printf("[ CHECK SERVER ] PCB criado com sucesso!\n");
    }

    ip_addr_t server_ip;
    server_ip.addr = ipaddr_addr(SERVER_IP);

    // Callback de recepção
    tcp_recv(pcb, recv_callback);

    if (tcp_connect(pcb, &server_ip, SERVER_PORT, NULL) != ERR_OK) {
        printf("[ CHECK SERVER ] Falha ao conectar ao servidor (/status)\n");
        tcp_abort(pcb);
        return;
    } else {
        printf("[ CHECK SERVER ] Conectado ao servidor (/status)\n");
    }

    // Montar requisição GET
    char request[256];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             SERVER_STATUS_PATH, SERVER_IP);

    tcp_sent(pcb, sent_callback_status);

    // Enviar requisição
    if (tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY) != ERR_OK) {
        printf("[ CHECK SERVER ] Erro ao escrever requisição GET\n");
        tcp_abort(pcb);
        return;
    } else {
        printf("[ CHECK SERVER ] Requisição GET escrita com sucesso!\n");
    }

    if (tcp_output(pcb) != ERR_OK) {
        printf("[ CHECK SERVER ] Erro ao enviar requisição GET\n");
        tcp_abort(pcb);
        return;
    } else {
        printf("[ CHECK SERVER ] Requisição GET enviada com sucesso!\n");
    }
}