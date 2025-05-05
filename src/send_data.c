#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "send_data.h"

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    // Callback para confirmação de envio
    return ERR_OK;
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    // Callback para recebimento de resposta (se necessário)
    if (p != NULL) {
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
    } else if (err == ERR_OK) {
        tcp_client_close(tpcb);
    }
    return ERR_OK;
}

static void tcp_client_err(void *arg, err_t err) {
    // Callback para tratamento de erros
    printf("[ ERRO ] Erro do cliente TCP: %d\n", err);
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("[ ERRO ] Falha na conexão: %d\n", err);
        return err;
    }

    // Configura callbacks
    tcp_sent(tpcb, tcp_client_sent);
    tcp_recv(tpcb, tcp_client_recv);
    tcp_err(tpcb, tcp_client_err);

    return ERR_OK;
}

static void tcp_client_close(struct tcp_pcb *tpcb) {
    if (tpcb != NULL) {
        tcp_arg(tpcb, NULL);
        tcp_sent(tpcb, NULL);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_close(tpcb);
    }
}

bool send_data_to_server(const char *data) {
    struct tcp_pcb *tpcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!tpcb) {
        printf("[ ERRO ] Falha ao criar o TCP PCB.\n");
        return false;
    }

    ip_addr_t server_addr;
    if (!ipaddr_aton(SERVER_IP, &server_addr)) {
        printf("[ ERRO ] IP do servidor inválido.\n");
        return false;
    }

    tcp_arg(tpcb, NULL);
    tcp_err(tpcb, tcp_client_err);

    if (tcp_connect(tpcb, &server_addr, SERVER_PORT, tcp_client_connected) != ERR_OK) {
        printf("[ ERRO ] Falha ao iniciar a conexão.\n");
        tcp_client_close(tpcb);
        return false;
    }

    // Aguarda conexão ser estabelecida
    sleep_ms(100);

    // Prepara o payload HTTP
    char http_request[512];
    snprintf(http_request, sizeof(http_request),
             "POST %s HTTP/1.1\r\n"
             "Host: %s:%d\r\n"
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             SERVER_PATH, SERVER_IP, SERVER_PORT, strlen(data), data);

    // Envia os dados
    err_t err = tcp_write(tpcb, http_request, strlen(http_request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK) {
        printf("[ ERRO ] Falha ao gravar dados: %d\n", err);
        tcp_client_close(tpcb);
        return false;
    }

    // Envia os dados imediatamente
    err = tcp_output(tpcb);
    if (err != ERR_OK) {
        printf("[ ERRO ] Falha ao enviar dados: %d\n", err);
        tcp_client_close(tpcb);
        return false;
    }

    // Aguarda o envio ser concluído
    sleep_ms(50);
    tcp_client_close(tpcb);
    return true;
}

void create_request(const char *direc, bool but_a, bool but_b, float db_le, int inten, float tempe) {

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
    
    if (!send_data_to_server(json_request)) {
        printf("[ ERRO ] Falha ao enviar dados para o servidor.\n");
    } else {
        printf("[ RESP ] Dados enviados com sucesso.\n");
    }
}