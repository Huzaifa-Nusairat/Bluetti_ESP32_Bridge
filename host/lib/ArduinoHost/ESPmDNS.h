#pragma once
#include "Arduino.h"
class MDNSResponder {
 public:
  bool begin(const char*) { return true; }
  void end() {}
  bool addService(const char*, const char*, uint16_t) { return true; }
};
extern MDNSResponder MDNS;
