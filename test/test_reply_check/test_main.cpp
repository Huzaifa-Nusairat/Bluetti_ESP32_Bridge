// Every BLE reply is checked before parse_bluetooth_data (Bluetti_ESP32/PayloadParser.cpp)
// uses it. Assumed reply layout, not yet confirmed on a real station: Modbus RTU, last two bytes = modbus_crc of every byte before them,
// low byte first. modbus_crc is pinned to the CRC-16/MODBUS check value 0x4B37 by test_frame_crc.
// Good frame: AC300 0x03 reply to the poll page 0x0B offset 0xF5, 7 registers
// (Device_AC300.h:115): 01 03 0E, 14 data bytes, CRC lo, CRC hi = 19 bytes.
// The only state field there is AUTO_SLEEP_MODE, UINT, 0x0B/0xF5 (Device_AC300.h:103);
// its first register 0x0002 parses to 2 (PayloadParser.cpp:8) and is published as
// "auto_sleep_mode -> 2" into the message view (MQTT.cpp:77, 369-370; BWifi.cpp:325-345).
#include <Arduino.h>
#include <unity.h>
#include <string.h>
#include "BWifi.h"
#include "PayloadParser.h"
#include "utils.h"

extern ESPBluettiSettings wifiConfig;  // BWifi.cpp:34
extern String lastMsg;                 // BWifi.cpp:19

static const size_t GOOD_LEN = 19;
static uint8_t frame[32];

static void build_good_frame(void) {
  memset(frame, 0, sizeof(frame));
  frame[0] = 0x01;            // address
  frame[1] = 0x03;            // function: read
  frame[2] = 0x0E;            // byte count: 7 registers
  frame[3] = 0x00;            // register 0x0BF5 = 0x0002 (AUTO_SLEEP_MODE)
  frame[4] = 0x02;
  uint16_t crc = modbus_crc(frame, GOOD_LEN - 2);
  frame[GOOD_LEN - 2] = (uint8_t)(crc & 0xFF);   // low byte first
  frame[GOOD_LEN - 1] = (uint8_t)(crc >> 8);
}

static bool published_auto_sleep(void) {
  return lastMsg.indexOf("auto_sleep_mode -> 2") >= 0;
}

void setUp(void) {
  wifiConfig.mqtt_server[0] = '\0';   // publishTopic goes to the message view only (MQTT.cpp:367-368)
  lastMsg = "";
  build_good_frame();
}
void tearDown(void) {}

// guard: a matching reply is accepted
void test_helper_accepts_good_frame(void) {
  TEST_ASSERT_TRUE(modbus_reply_crc_ok(frame, GOOD_LEN));
}

void test_helper_rejects_flipped_crc_byte(void) {
  frame[GOOD_LEN - 2] ^= 0x01;
  TEST_ASSERT_FALSE(modbus_reply_crc_ok(frame, GOOD_LEN));
}

void test_helper_rejects_swapped_crc_bytes(void) {
  uint8_t lo = frame[GOOD_LEN - 2];
  uint8_t hi = frame[GOOD_LEN - 1];
  TEST_ASSERT_NOT_EQUAL(lo, hi);       // a swap must change the frame
  frame[GOOD_LEN - 2] = hi;
  frame[GOOD_LEN - 1] = lo;
  TEST_ASSERT_FALSE(modbus_reply_crc_ok(frame, GOOD_LEN));
}

void test_helper_rejects_lengths_0_to_4(void) {
  for (size_t len = 0; len < 5; len++) {
    uint8_t buf[8];
    memset(buf, 0, sizeof(buf));
    buf[0] = 0x01;
    buf[1] = 0x03;
    if (len >= 2) {                     // give the short frame a matching CRC where it has room
      uint16_t crc = modbus_crc(buf, (int)(len - 2));
      buf[len - 2] = (uint8_t)(crc & 0xFF);
      buf[len - 1] = (uint8_t)(crc >> 8);
    }
    TEST_ASSERT_FALSE_MESSAGE(modbus_reply_crc_ok(buf, len), "short reply accepted");
  }
}

// guard: the boundary of "shorter than 5 bytes" (request): a 5-byte reply with a matching CRC
// (Modbus exception layout 01 83 02 CRC-lo CRC-hi) is accepted
void test_helper_accepts_5_byte_reply(void) {
  uint8_t buf[8] = {0x01, 0x83, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint16_t crc = modbus_crc(buf, 3);
  buf[3] = (uint8_t)(crc & 0xFF);
  buf[4] = (uint8_t)(crc >> 8);
  TEST_ASSERT_TRUE(modbus_reply_crc_ok(buf, 5));
}

// guard: published as today; runs the field copy loop (PayloadParser.cpp:83) under the sanitizers
void test_parse_good_frame_publishes(void) {
  parse_bluetooth_data(0x0B, 0xF5, frame, GOOD_LEN);
  TEST_ASSERT_TRUE(published_auto_sleep());
}

void test_parse_flipped_crc_publishes_nothing(void) {
  frame[GOOD_LEN - 1] ^= 0x80;
  parse_bluetooth_data(0x0B, 0xF5, frame, GOOD_LEN);
  TEST_ASSERT_EQUAL_STRING("", lastMsg.c_str());
}

void test_parse_swapped_crc_publishes_nothing(void) {
  uint8_t lo = frame[GOOD_LEN - 2];
  frame[GOOD_LEN - 2] = frame[GOOD_LEN - 1];
  frame[GOOD_LEN - 1] = lo;
  parse_bluetooth_data(0x0B, 0xF5, frame, GOOD_LEN);
  TEST_ASSERT_EQUAL_STRING("", lastMsg.c_str());
}

// guard: the field bounds check already blocks these at the pin
void test_parse_short_publishes_nothing(void) {
  for (size_t len = 0; len < 5; len++) {
    uint8_t buf[8] = {0x01, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
    lastMsg = "";
    parse_bluetooth_data(0x0B, 0xF5, buf, len);
    TEST_ASSERT_EQUAL_STRING("", lastMsg.c_str());
  }
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_helper_accepts_good_frame);
  RUN_TEST(test_helper_rejects_flipped_crc_byte);
  RUN_TEST(test_helper_rejects_swapped_crc_bytes);
  RUN_TEST(test_helper_rejects_lengths_0_to_4);
  RUN_TEST(test_helper_accepts_5_byte_reply);
  RUN_TEST(test_parse_good_frame_publishes);
  RUN_TEST(test_parse_flipped_crc_publishes_nothing);
  RUN_TEST(test_parse_swapped_crc_publishes_nothing);
  RUN_TEST(test_parse_short_publishes_nothing);
  UNITY_END();
}
void loop() {}
