// Reproduction: callback()
// (Bluetti_ESP32/MQTT.cpp:286-317) on an unmatched and on a matched topic.
// Expected: an unmatched topic queues no frame;
// a matched topic queues exactly the frame it queues today (guard).
#include <Arduino.h>
#include <unity.h>
#include "BTooth.h"
#include "utils.h"

extern QueueHandle_t sendQueue;                       // BTooth.cpp:18
void callback(char* topic, byte* payload, unsigned int length);  // MQTT.cpp:286, not in MQTT.h

void setUp(void) {
  sendQueue = xQueueCreate(5, sizeof(bt_command_t));  // as BTooth.cpp:77; fresh per case
}
void tearDown(void) {}

void test_unmatched_topic_queues_no_frame(void) {
  char topic[] = "bluetti/test/command/not_a_field";
  byte payload[3] = {'O', 'N', 0};                    // one byte to spare (MQTT.cpp:287)
  TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(sendQueue));
  callback(topic, payload, 2);
  TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(sendQueue));
}

// Guard: AC_OUTPUT_ON row of bluetti_device_command in Device_AC300.h:108 = page 0x0B, offset 0xBF
void test_matched_topic_queues_one_frame(void) {
  char topic[] = "bluetti/test/command/ac_output_on";
  byte payload[3] = {'O', 'N', 0};
  TEST_ASSERT_EQUAL_UINT32(0, uxQueueMessagesWaiting(sendQueue));
  callback(topic, payload, 2);
  TEST_ASSERT_EQUAL_UINT32(1, uxQueueMessagesWaiting(sendQueue));
  bt_command_t f;
  TEST_ASSERT_EQUAL(pdTRUE, xQueueReceive(sendQueue, &f, 0));
  TEST_ASSERT_EQUAL_HEX8(0x01, f.prefix);
  TEST_ASSERT_EQUAL_HEX8(0x06, f.field_update_cmd);
  TEST_ASSERT_EQUAL_HEX8(0x0B, f.page);
  TEST_ASSERT_EQUAL_HEX8(0xBF, f.offset);
  TEST_ASSERT_EQUAL_HEX16(swap_bytes(1), f.len);
  TEST_ASSERT_EQUAL_HEX16(modbus_crc((uint8_t*)&f, 6), f.check_sum);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_unmatched_topic_queues_no_frame);
  RUN_TEST(test_matched_topic_queues_one_frame);
  UNITY_END();
}
void loop() {}
