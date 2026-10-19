// Native tests for the reply-field parsers in Bluetti_ESP32/PayloadParser.cpp.
// Each expected value is the result of the code line named in the case's comment, at the pinned commit d7eae2f.
// Out of scope (not tested here): parse_bluetooth_data, parse_string_field, parse_enum_field.
#include <Arduino.h>
#include <unity.h>
#include "PayloadParser.h"

void setUp(void) {}
void tearDown(void) {}

// PayloadParser.cpp:8 -- (data[0] << 8) | data[1]
void test_uint_field_is_big_endian(void) {
  uint8_t data[2] = {0x12, 0x34};
  TEST_ASSERT_EQUAL_HEX16(0x1234, parse_uint_field(data));
}

// PayloadParser.cpp:12 -- data[1] == 1
void test_bool_field_true_when_second_byte_is_one(void) {
  uint8_t data[2] = {0x00, 0x01};
  TEST_ASSERT_TRUE(parse_bool_field(data));
}

// PayloadParser.cpp:12 -- data[0] is not read
void test_bool_field_ignores_first_byte(void) {
  uint8_t data[2] = {0xFF, 0x01};
  TEST_ASSERT_TRUE(parse_bool_field(data));
}

// PayloadParser.cpp:12 -- any second byte other than 1 is false (guard: passes on the stub too)
void test_bool_field_false_when_second_byte_is_two(void) {
  uint8_t data[2] = {0x00, 0x02};
  TEST_ASSERT_FALSE(parse_bool_field(data));
}

// PayloadParser.cpp:16-17 -- raw 0x012C = 300, divided by pow(10, scale); 300, 30 and 3 are exact in float
void test_decimal_field_scale_0(void) {
  uint8_t data[2] = {0x01, 0x2C};
  TEST_ASSERT_TRUE(parse_decimal_field(data, 0) == 300.0f);
}

void test_decimal_field_scale_1(void) {
  uint8_t data[2] = {0x01, 0x2C};
  TEST_ASSERT_TRUE(parse_decimal_field(data, 1) == 30.0f);
}

void test_decimal_field_scale_2(void) {
  uint8_t data[2] = {0x01, 0x2C};
  TEST_ASSERT_TRUE(parse_decimal_field(data, 2) == 3.0f);
}

// PayloadParser.cpp:22-26 -- low = first register (100), high = second (0): (100 | 0 << 16) / 100 = 1.0
void test_version_field_low_word_first(void) {
  uint8_t data[4] = {0x00, 0x64, 0x00, 0x00};
  TEST_ASSERT_TRUE(parse_version_field(data) == 1.0f);
}

// PayloadParser.cpp:22-26 -- high = second register (0x0019 = 25): (25 << 16) / 100 = 1638400 / 100 = 16384.0
// data[2] < 0x80 keeps high << 16 inside int (PayloadParser.cpp:24)
void test_version_field_second_register_is_high_word(void) {
  uint8_t data[4] = {0x00, 0x00, 0x00, 0x19};
  TEST_ASSERT_TRUE(parse_version_field(data) == 16384.0f);
}

// PayloadParser.cpp:31-36 -- val1 | val2 << 16 | val3 << 32 | val4 << 48, first register least significant
void test_serial_field_low_word_first(void) {
  uint8_t data[8] = {0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04};
  TEST_ASSERT_TRUE(parse_serial_field(data) == 0x0004000300020001ULL);
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_uint_field_is_big_endian);
  RUN_TEST(test_bool_field_true_when_second_byte_is_one);
  RUN_TEST(test_bool_field_ignores_first_byte);
  RUN_TEST(test_bool_field_false_when_second_byte_is_two);
  RUN_TEST(test_decimal_field_scale_0);
  RUN_TEST(test_decimal_field_scale_1);
  RUN_TEST(test_decimal_field_scale_2);
  RUN_TEST(test_version_field_low_word_first);
  RUN_TEST(test_version_field_second_register_is_high_word);
  RUN_TEST(test_serial_field_low_word_first);
  UNITY_END();
}

void loop() {}
