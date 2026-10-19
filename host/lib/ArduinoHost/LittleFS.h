// Host stand-in for LittleFS: a real folder. HOST_TEST_FS names it; otherwise a fresh temporary folder per run.
#pragma once
#include "FS.h"

class LittleFSClass : public fs::FS {
 public:
  LittleFSClass() : fs::FS("HOST_TEST_FS") {}
  bool begin(bool formatOnFail = false, const char* basePath = "/littlefs", uint8_t maxOpen = 10, const char* label = nullptr);
  void end() {}
  bool format();
  size_t totalBytes() { return 1024u * 1024u; }
  size_t usedBytes();
};
extern LittleFSClass LittleFS;
