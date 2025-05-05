#include "lwip/tcp.h"
#include "lwip/ip_addr.h"
#include "pico/cyw43_arch.h"

// #define SERVER_IP "192.168.1.80"
// #define SERVER_PORT 3000
// #define SERVER_PATH "/receber"

#define SERVER_IP "192.168.1.80"
#define SERVER_PORT 3000
#define SERVER_PATH "/receber"

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_client_err(void *arg, err_t err);
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void tcp_client_close(struct tcp_pcb *tpcb);
bool send_data_to_server(const char *data);
void create_request(const char *direc, bool but_a, bool but_b, float db_le, int inten, float tempe);