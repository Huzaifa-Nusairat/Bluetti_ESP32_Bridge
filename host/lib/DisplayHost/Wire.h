// Host stand-in for the Arduino-ESP32 Wire (I2C) library: no bus; begin() records the pins.
#pragma once
#include "Arduino.h"
class TwoWire {
 public:
  bool begin(int sda = -1, int scl = -1, uint32_t frequency = 0) { sda_ = sda; scl_ = scl; (void)frequency; return true; }
  void setClock(uint32_t) {}
  int host_sda() const { return sda_; }
  int host_scl() const { return scl_; }
 private:
  int sda_ = -1, scl_ = -1;
};
extern TwoWire Wire;
