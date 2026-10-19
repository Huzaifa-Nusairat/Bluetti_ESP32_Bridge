// Native tests for the frame checksum and byte order in Bluetti_ESP32/utils.cpp.
// crc16_update (crc16.h:41) is reached through modbus_crc only; crc16.h is not included here.
#include <Arduino.h>
#include <unity.h>
#include "utils.h"

void setUp(void) {}
void tearDown(void) {}

// utils.cpp:9-15 -- init 0xFFFF, crc16_update per byte: the CRC-16/MODBUS check value over "123456789" is 0x4B37
// (value from the request, inputs/00-request.json)
void test_modbus_crc_check_value(void) {
  uint8_t buf[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
  TEST_ASSERT_EQUAL_HEX16(0x4B37, modbus_crc(buf, 9));
}

// utils.cpp:9-15 -- len 0: the loop does not run, the initial value 0xFFFF is returned
void test_modbus_crc_empty_buffer(void) {
  uint8_t buf[1] = {0x00};
  TEST_ASSERT_EQUAL_HEX16(0xFFFF, modbus_crc(buf, 0));
}

// utils.cpp:5 -- (number << 8) | (number >> 8), truncated to 16 bits
void test_swap_bytes_1234(void) {
  TEST_ASSERT_EQUAL_HEX16(0x3412, swap_bytes(0x1234));
}

void test_swap_bytes_00ff(void) {
  TEST_ASSERT_EQUAL_HEX16(0xFF00, swap_bytes(0x00FF));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_modbus_crc_check_value);
  RUN_TEST(test_modbus_crc_empty_buffer);
  RUN_TEST(test_swap_bytes_1234);
  RUN_TEST(test_swap_bytes_00ff);
  UNITY_END();
}

void loop() {}
