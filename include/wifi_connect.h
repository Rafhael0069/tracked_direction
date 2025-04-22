#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// #define WIFI_SSID ""
// #define WIFI_PASSWORD ""

#define WIFI_SSID getenv("WIFI_SSID")
#define WIFI_PASSWORD getenv("WIFI_PASSWORD")

void init_wifi();