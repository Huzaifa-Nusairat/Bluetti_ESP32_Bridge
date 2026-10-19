// Host stand-in for the Arduino-ESP32 EEPROM library: a 4096-byte memory that lives as long as the test program.
// Like the board, begin(size) maps the first `size` bytes, writes land in memory and are kept by commit(); a write
// after end() and before begin() is refused. A fresh board reads 0xFF (host_eeprom_erase()).
#pragma once
#include <string.h>
#include "Arduino.h"

class EEPROMClass {
 public:
  static const size_t kMax = 4096;
  bool begin(size_t size);
  void end() { open_ = false; }
  bool commit();
  uint8_t read(int address) { return in(address, 1) ? ram_[address] : 0; }
  void write(int address, uint8_t value) { if (open_ && in(address, 1)) ram_[address] = value; }
  template <typename T> T& get(int address, T& t) {
    if (in(address, sizeof(T))) memcpy(&t, ram_ + address, sizeof(T));
    return t;
  }
  template <typename T> const T& put(int address, const T& t) {
    if (open_ && in(address, sizeof(T))) memcpy(ram_ + address, &t, sizeof(T));
    return t;
  }
  size_t length() { return size_; }
  // Host-only.
  const uint8_t* host_flash() const { return flash_; }
  int host_commits() const { return commits_; }
  void host_erase();
 private:
  bool in(int address, size_t n) const { return address >= 0 && (size_t)address + n <= size_; }
  uint8_t ram_[kMax];
  uint8_t flash_[kMax];
  size_t size_ = 0;
  bool open_ = false;
  bool init_ = false;
  int commits_ = 0;
};
extern EEPROMClass EEPROM;
void host_eeprom_erase();
