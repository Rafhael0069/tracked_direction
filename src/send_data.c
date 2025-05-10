#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"  // Biblioteca do Wi-Fi do Pico W
#include "lwip/pbuf.h"        // Buffer da pilha de rede LWIP
#include "lwip/tcp.h"         // Funções para comunicação TCP
#include "send_data.h"        // Header deste módulo

/** Callback chamado quando os dados enviados foram confirmados pelo servidor */
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    // Neste exemplo, não faz nada além de confirmar que foi enviado
    return ERR_OK;
}

/** Callback chamado quando o cliente recebe uma resposta do servidor */
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        tcp_recved(tpcb, p->tot_len);  // Informa à pilha TCP que os dados foram recebidos
        pbuf_free(p);                  // Libera o buffer de memória
    } else if (err == ERR_OK) {        // Se p é NULL e não houve erro, a conexão foi fechada
        tcp_client_close(tpcb);        // Fecha a conexão TCP
    }
    return ERR_OK;
}

/** Callback chamado quando ocorre algum erro durante a conexão TCP */
static void tcp_client_err(void *arg, err_t err) {
    printf("[ ERRO ] Erro do cliente TCP: %d\n", err); // Exibe o código de erro
}

/** Callback chamado quando a conexão TCP é estabelecida com sucesso */
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("[ ERRO ] Falha na conexão: %d\n", err); // Exibe o erro caso a conexão falhe
        return err;
    }

    // Configura os callbacks para envio, recebimento e erro
    tcp_sent(tpcb, tcp_client_sent);
    tcp_recv(tpcb, tcp_client_recv);
    tcp_err(tpcb, tcp_client_err);

    return ERR_OK;
}

/** Função que fecha e limpa a conexão TCP */
static void tcp_client_close(struct tcp_pcb *tpcb) {
    if (tpcb != NULL) {
        // Remove todos os callbacks e argumentos associados
        tcp_arg(tpcb, NULL);
        tcp_sent(tpcb, NULL);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_close(tpcb);  // Fecha a conexão
    }
}

/** Função principal que envia dados JSON para o servidor via TCP */
bool send_data_to_server(const char *data) {
    // Cria uma nova estrutura para conexão TCP
    struct tcp_pcb *tpcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!tpcb) {
        printf("[ ERRO ] Falha ao criar o TCP PCB.\n");
        return false;
    }

    // Converte o IP do servidor para estrutura ip_addr_t
    ip_addr_t server_addr;
    if (!ipaddr_aton(SERVER_IP, &server_addr)) {
        printf("[ ERRO ] IP do servidor inválido.\n");
        return false;
    }

    // Configura callbacks para erros
    tcp_arg(tpcb, NULL);
    tcp_err(tpcb, tcp_client_err);

    // Inicia a conexão TCP com o servidor
    if (tcp_connect(tpcb, &server_addr, SERVER_PORT, tcp_client_connected) != ERR_OK) {
        printf("[ ERRO ] Falha ao iniciar a conexão.\n");
        tcp_client_close(tpcb); // Fecha a conexão se falhar
        return false;
    }

    sleep_ms(100); // Aguarda conexão ser estabelecida (recurso simples, mas não ideal)

    // Monta o payload com a requisição HTTP POST
    char http_request[512];
    snprintf(http_request, sizeof(http_request),
             "POST %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             SERVER_PATH, SERVER_IP, SERVER_PORT, strlen(data), data);

    // Envia os dados via TCP
    err_t err = tcp_write(tpcb, http_request, strlen(http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("[ ERRO ] Falha ao gravar dados: %d\n", err);
        tcp_client_close(tpcb); // Fecha a conexão se falhar
        return false;
    }

    // Solicita envio imediato dos dados
    err = tcp_output(tpcb);
    if (err != ERR_OK) {
        printf("[ ERRO ] Falha ao enviar dados: %d\n", err);
        tcp_client_close(tpcb); // Fecha a conexão se falhar
        return false;
    }

    sleep_ms(50); // Aguarda o envio concluir
    tcp_client_close(tpcb);  // Fecha a conexão
    return true;
}

/** Função que cria o JSON e chama a função de envio */
void create_request(const char *direc, bool but_a, bool but_b, float db_le, int inten, float tempe) {
    // Monta a string JSON com os valores recebidos
    char json_request[512];
    snprintf(json_request, sizeof(json_request),
            "{"
            "\"direc\": \"%s\", "
            "\"but_a\": \"%s\", "
            "\"but_b\": \"%s\", "
            "\"db_le\": %.2f, "
            "\"inten\": %d, "
            "\"tempe\": %.2f"
            "}",
            direc,
            but_a ? "Pressionado" : "Solto",
            but_b ? "Pressionado" : "Solto",
            db_le,
            inten,
            tempe
        );
    
    // Envia a requisição JSON ao servidor
    if (!send_data_to_server(json_request)) {
        printf("[ ERRO ] Falha ao enviar dados para o servidor.\n");
    } else {
        printf("[ RESP ] Dados enviados com sucesso.\n");
    }
}