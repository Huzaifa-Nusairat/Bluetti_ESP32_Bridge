# Change record: check each Bluetooth LE reply's CRC before it is used

Commit `d26421e` on branch `verified-changes`. Firmware files: `Bluetti_ESP32/PayloadParser.cpp`,
`Bluetti_ESP32/utils.cpp`, `Bluetti_ESP32/utils.h`. Test: `test/test_reply_check`.

| Check | Result | How to reproduce |
| --- | --- | --- |
| Build | Passed for `esp32dev`; flash +368 bytes against the commit before | `pio run -e esp32dev` on this commit and on `d26421e~1`, compare the reported flash use |
| Native tests | 25 of 25 cases pass; address and undefined-behaviour sanitizers clean | `pio test -e native`, or the sanitized command in `host/README.md` |
| Mutation | 13 of 14 mutants caught; the one survivor is caught only by the sanitized run (table below) | `host/mutate.py host/mutants/reply-crc.json <unity-src> --markdown` |
| Review | One round of changes requested, then approved: state the unconfirmed reply layout as an assumption, and add a test that accepts the shortest valid reply (5 bytes) | the commit as it stands carries both |
| Bench | **Open.** The station's reply layout and CRC byte order are not yet confirmed on real hardware | needs a Bluetti station; not run |

## Native tests

```
PASS test_frame_crc: 4 Tests 0 Failures 0 Ignored 
PASS test_payload_fields: 10 Tests 0 Failures 0 Ignored 
PASS test_reply_check: 9 Tests 0 Failures 0 Ignored 
PASS test_unmatched_topic: 2 Tests 0 Failures 0 Ignored 
suites passed: 4, failed: 0
```

## Mutation

Each mutant changes one token on a line this commit added or changed. "Caught" means at least one
native test failed with the mutant in place.

13 of 14 mutants caught by the plain run; 14 of 14 with the sanitized run.

| File:line | Mutation | Result |
| --- | --- | --- |
| `Bluetti_ESP32/PayloadParser.cpp:89` | `for (int i=data_start; i< data_end; i++){` → `for (int i=data_start; i<= data_end; i++){` | caught only by the sanitized run |
| `Bluetti_ESP32/PayloadParser.cpp:89` | `for (int i=data_start; i< data_end; i++){` → `for (int i=data_start; i< data_end; i--){` | caught |
| `Bluetti_ESP32/utils.cpp:19` | `if (buf == NULL \|\| len < 5) {` → `if (buf != NULL \|\| len < 5) {` | caught |
| `Bluetti_ESP32/utils.cpp:19` | `if (buf == NULL \|\| len < 5) {` → `if (buf == NULL && len < 5) {` | caught |
| `Bluetti_ESP32/utils.cpp:19` | `if (buf == NULL \|\| len < 5) {` → `if (buf == NULL \|\| len <= 5) {` | caught |
| `Bluetti_ESP32/utils.cpp:19` | `if (buf == NULL \|\| len < 5) {` → `if (buf == NULL \|\| len < 6) {` | caught |
| `Bluetti_ESP32/utils.cpp:22` | `uint16_t expected = modbus_crc((uint8_t*)buf, (int)(len - 2));` → `uint16_t expected = modbus_crc((uint8_t*)buf, (int)(len + 2));` | caught |
| `Bluetti_ESP32/utils.cpp:22` | `uint16_t expected = modbus_crc((uint8_t*)buf, (int)(len - 2));` → `uint16_t expected = modbus_crc((uint8_t*)buf, (int)(len - 3));` | caught |
| `Bluetti_ESP32/utils.cpp:23` | `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` → `uint16_t received = (uint16_t)buf[len + 2] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` | caught |
| `Bluetti_ESP32/utils.cpp:23` | `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` → `uint16_t received = (uint16_t)buf[len - 3] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` | caught |
| `Bluetti_ESP32/utils.cpp:23` | `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` → `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len + 1] << 8);  // low byte first` | caught |
| `Bluetti_ESP32/utils.cpp:23` | `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` → `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 2] << 8);  // low byte first` | caught |
| `Bluetti_ESP32/utils.cpp:23` | `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 1] << 8);  // low byte first` → `uint16_t received = (uint16_t)buf[len - 2] \| ((uint16_t)buf[len - 1] << 9);  // low byte first` | caught |
| `Bluetti_ESP32/utils.cpp:24` | `return expected == received;` → `return expected != received;` | caught |

The survivor writes one byte past the field buffer. No plain test can see that; the sanitized run
stops on it, which is why the sanitized command is part of this record.
