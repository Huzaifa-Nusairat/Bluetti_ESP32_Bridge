// Host stand-in for the ESP-IDF sleep calls the firmware makes. Deep sleep never returns on the board; on the host
// esp_deep_sleep_start() records the request and returns, so a test can see it (host_deep_sleeps()).
#pragma once
#include <stdint.h>
#include "esp_system.h"
esp_err_t esp_sleep_enable_timer_wakeup(uint64_t time_in_us);
void esp_deep_sleep_start(void);
uint64_t host_sleep_timer_us();
int host_deep_sleeps();
