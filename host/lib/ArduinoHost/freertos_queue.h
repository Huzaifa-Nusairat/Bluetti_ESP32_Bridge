// Host stand-in for the FreeRTOS queue API the firmware uses (on the board it arrives through Arduino.h and NimBLE).
// A real FIFO of fixed-size items; timeouts are ignored (nothing else runs on the host), a full queue refuses.
#pragma once
#include <stddef.h>
#include <stdint.h>
typedef int BaseType_t;
typedef uint32_t TickType_t;
#define pdTRUE 1
#define pdFALSE 0
#define pdPASS pdTRUE
#define portMAX_DELAY ((TickType_t)0xffffffffUL)
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
struct host_queue;
typedef host_queue* QueueHandle_t;
QueueHandle_t xQueueCreate(size_t length, size_t item_size);
BaseType_t xQueueSend(QueueHandle_t q, const void* item, TickType_t wait);
BaseType_t xQueueReceive(QueueHandle_t q, void* item, TickType_t wait);
size_t uxQueueMessagesWaiting(QueueHandle_t q);
void vQueueDelete(QueueHandle_t q);
void vTaskDelay(TickType_t ticks);
