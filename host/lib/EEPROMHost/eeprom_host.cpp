#include "EEPROM.h"
EEPROMClass EEPROM;
bool EEPROMClass::begin(size_t size) {
  if (!init_) { memset(flash_, 0xFF, kMax); init_ = true; }
  if (size == 0 || size > kMax) return false;
  size_ = size;
  memcpy(ram_, flash_, size_);  // what the flash holds now
  open_ = true;
  return true;
}
bool EEPROMClass::commit() {
  if (!open_) return false;
  memcpy(flash_, ram_, size_);
  ++commits_;
  return true;
}
void EEPROMClass::host_erase() { memset(flash_, 0xFF, kMax); memset(ram_, 0xFF, kMax); init_ = true; size_ = 0; open_ = false; commits_ = 0; }
void host_eeprom_erase() { EEPROM.host_erase(); }
