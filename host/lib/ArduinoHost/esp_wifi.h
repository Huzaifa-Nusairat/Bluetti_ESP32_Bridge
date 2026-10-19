// Host stand-in for the ESP-IDF Wi-Fi power-save call the firmware makes: recorded, nothing else.
#pragma once
#include "esp_system.h"
typedef enum { WIFI_PS_NONE, WIFI_PS_MIN_MODEM, WIFI_PS_MAX_MODEM } wifi_ps_type_t;
esp_err_t esp_wifi_set_ps(wifi_ps_type_t type);
wifi_ps_type_t host_wifi_ps();
