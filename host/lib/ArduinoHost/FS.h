// Host stand-in for the ESP32 FS / File API, backed by a real folder (see LittleFS.h).
#pragma once
#include <memory>
#include <string>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "Print.h"

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

namespace fs {

enum SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2 };

class File : public Stream {
 public:
  File() {}
  File(const std::string& host, const std::string& name, const char* mode);
  explicit operator bool() const { return st_ && (st_->fp || st_->dir); }
  size_t write(uint8_t c) override { return write(&c, 1); }
  size_t write(const uint8_t* b, size_t n) override {
    size_t k = st_ && st_->fp ? fwrite(b, 1, n, st_->fp) : 0;
    if (k != n) setWriteError();
    return k;
  }
  using Print::write;
  int available() override;
  int read() override { return st_ && st_->fp ? fgetc(st_->fp) : -1; }
  size_t read(uint8_t* buf, size_t n) { return st_ && st_->fp ? fread(buf, 1, n, st_->fp) : 0; }
  int peek() override;
  void flush() override { if (st_ && st_->fp) fflush(st_->fp); }
  bool seek(uint32_t pos, SeekMode mode = SeekSet) { return st_ && st_->fp && fseek(st_->fp, (long)pos, (int)mode) == 0; }
  size_t position() const { return st_ && st_->fp ? (size_t)ftell(st_->fp) : 0; }
  size_t size() const;
  void close() { if (st_) st_->close(); }
  const char* name() const;
  const char* path() const { return st_ ? st_->name.c_str() : ""; }
  bool isDirectory() const { return st_ && st_->dir; }
  File openNextFile(const char* mode = FILE_READ);
  time_t getLastWrite() const { return 0; }

 private:
  struct State {
    std::string host, name; FILE* fp = nullptr; DIR* dir = nullptr;
    void close() { if (fp) { fclose(fp); fp = nullptr; } if (dir) { closedir(dir); dir = nullptr; } }
    ~State() { close(); }
  };
  std::shared_ptr<State> st_;
};

class FS {
 public:
  explicit FS(const char* root_env) : root_env_(root_env) {}
  File open(const char* path, const char* mode = FILE_READ, bool create = false);
  File open(const String& path, const char* mode = FILE_READ, bool create = false) { return open(path.c_str(), mode, create); }
  bool exists(const char* path);
  bool exists(const String& path) { return exists(path.c_str()); }
  bool remove(const char* path);
  bool remove(const String& path) { return remove(path.c_str()); }
  bool rename(const char* from, const char* to);
  bool rename(const String& from, const String& to) { return rename(from.c_str(), to.c_str()); }
  bool mkdir(const char* path);
  bool mkdir(const String& path) { return mkdir(path.c_str()); }
  bool rmdir(const char* path);
  std::string host(const char* path);

 protected:
  const char* root_env_;
  std::string root_;
};

}  // namespace fs

using fs::File;
using fs::FS;
