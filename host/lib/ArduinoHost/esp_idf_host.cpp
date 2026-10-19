#include "esp_wifi.h"
#include "esp_sleep.h"
namespace { wifi_ps_type_t g_ps = WIFI_PS_NONE; uint64_t g_timer = 0; int g_sleeps = 0; }
esp_err_t esp_wifi_set_ps(wifi_ps_type_t type) { g_ps = type; return ESP_OK; }
wifi_ps_type_t host_wifi_ps() { return g_ps; }
esp_err_t esp_sleep_enable_timer_wakeup(uint64_t us) { g_timer = us; return ESP_OK; }
void esp_deep_sleep_start(void) { ++g_sleeps; }
uint64_t host_sleep_timer_us() { return g_timer; }
int host_deep_sleeps() { return g_sleeps; }
