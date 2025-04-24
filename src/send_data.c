#include "send_data.h"

#define SERVER_IP "192.168.1.168"        // ip do servidor
#define SERVER_PORT 3000                // porta do servidor
#define SERVER_PATH "/receber"          // rota de envio de dados

#define TCP_TIMEOUT_MS 5000
#define MAX_RETRIES 3

// Callback para quando os dados são enviados
static err_t sent_callback(void *arg, struct tcp_pcb *pcb, u16_t len) {
    printf("[ RESP ] Dados enviados com sucesso!\n");
    tcp_close(pcb); // Fecha a conexão TCP
    return ERR_OK;
}

// Envia dados para o servidor
void send_data_to_server(const char *path, char *request_body, const char *type_method) {
    // Criando PCB
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("[ ERRO ] Erro ao criar PCB\n");
        return;
    } 
    // else {
    //     printf("[ INFO ] PCB criado com sucesso\n");
    // }

    // Organizando o endereço IP
    ip_addr_t server_ip;
    server_ip.addr = ipaddr_addr(SERVER_IP);

    // Conectando ao servidor
    if (tcp_connect(pcb, &server_ip, SERVER_PORT, NULL) != ERR_OK) {
        printf("[ ERRO ] Erro ao conectar ao servidor\n");
        tcp_abort(pcb);
        return;
    } 
    // else {
    //     printf("[ INFO ] Conectado ao servidor %s:%d\n", SERVER_IP, SERVER_PORT);
    // }

    // Montando requisição
    char request[521];
    snprintf(request, sizeof(request),
             "%s %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             type_method, path, SERVER_IP, strlen(request_body), request_body);

    // Definindo o callback para quando os dados forem enviados com sucesso
    tcp_sent(pcb, sent_callback);

    // Empacotando a requisição
    if (tcp_write(pcb, request, strlen(request), TCP_WRITE_FLAG_COPY) != ERR_OK) {
        printf("[ ERRO ] Erro ao enviar dados\n");
        tcp_abort(pcb);
        return;
    } 
    // else {
    //     printf("[ INFO ] Dados enviados: %s\n", request);
    // }

    // Enviando a requisição
    if (tcp_output(pcb) != ERR_OK) {
        printf("[ ERRO ] Erro ao enviar dados (tcp_output)\n");
        tcp_abort(pcb);
        return;
    } 
    // else {
    //     printf("[ INFO ] Dados enviados com sucesso (tcp_output)\n");
    // }
}

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