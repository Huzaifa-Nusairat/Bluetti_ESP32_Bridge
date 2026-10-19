#include "freertos_queue.h"
#include <string.h>
#include <deque>
#include <vector>
#include "Arduino.h"
struct host_queue {
  size_t length, item;
  std::deque<std::vector<uint8_t>> items;
};
QueueHandle_t xQueueCreate(size_t length, size_t item_size) { return new host_queue{length, item_size, {}}; }
BaseType_t xQueueSend(QueueHandle_t q, const void* item, TickType_t) {
  if (!q || q->items.size() >= q->length) return pdFALSE;
  const uint8_t* p = static_cast<const uint8_t*>(item);
  q->items.emplace_back(p, p + q->item);
  return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t q, void* item, TickType_t) {
  if (!q || q->items.empty()) return pdFALSE;
  memcpy(item, q->items.front().data(), q->item);
  q->items.pop_front();
  return pdTRUE;
}
size_t uxQueueMessagesWaiting(QueueHandle_t q) { return q ? q->items.size() : 0; }
void vQueueDelete(QueueHandle_t q) { delete q; }
void vTaskDelay(TickType_t ticks) { delay(ticks); }
