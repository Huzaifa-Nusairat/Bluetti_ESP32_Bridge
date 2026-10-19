// Host stand-in for Arduino's Print (output is captured, never printed, so test output stays Unity's).
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <string>
#include "WString.h"

#define DEC 10
#define HEX 16
#define OCT 8
#define BIN 2

class Print;
class Printable {
 public:
  virtual ~Printable() {}
  virtual size_t printTo(Print& p) const = 0;
};

class Print {
 public:
  virtual ~Print() {}
  virtual size_t write(uint8_t c) = 0;
  virtual size_t write(const uint8_t* buf, size_t n) { size_t k = 0; while (n--) k += write(*buf++); return k; }
  size_t write(const char* s) { return s ? write(reinterpret_cast<const uint8_t*>(s), strlen(s)) : 0; }
  size_t print(const char* s) { return write(s); }
  size_t print(const String& s) { return write(s.c_str()); }
  size_t print(char c) { return write(static_cast<uint8_t>(c)); }
  size_t print(int v, int base = DEC) { return print(String(v, (unsigned char)base)); }
  size_t print(unsigned int v, int base = DEC) { return print(String(v, (unsigned char)base)); }
  size_t print(long v, int base = DEC) { return print(String(v, (unsigned char)base)); }
  size_t print(unsigned long v, int base = DEC) { return print(String(v, (unsigned char)base)); }
  size_t print(double v, int digits = 2) { return print(String(v, (unsigned int)digits)); }
  size_t print(const Printable& x) { return x.printTo(*this); }
  template <typename T> size_t println(const T& v) { return print(v) + println(); }
  template <typename T> size_t println(const T& v, int x) { return print(v, x) + println(); }
  size_t println() { return write("\r\n"); }
  size_t printf(const char* f, ...) __attribute__((format(printf, 2, 3))) {
    char buf[1024];
    va_list ap; va_start(ap, f); int n = vsnprintf(buf, sizeof buf, f, ap); va_end(ap);
    return n > 0 ? write(reinterpret_cast<const uint8_t*>(buf), strlen(buf)) : 0;
  }
  virtual void flush() {}
  int getWriteError() { return write_error_; }
  void clearWriteError() { write_error_ = 0; }

 protected:
  void setWriteError(int err = 1) { write_error_ = err; }

 private:
  int write_error_ = 0;
};

class Stream : public Print {
 public:
  virtual int available() { return 0; }
  virtual int read() { return -1; }
  virtual int peek() { return -1; }
  String readStringUntil(char term) {
    String out; int c;
    while ((c = read()) >= 0 && c != term) out += static_cast<char>(c);
    return out;
  }
  String readString() { String out; int c; while ((c = read()) >= 0) out += static_cast<char>(c); return out; }
  size_t readBytes(uint8_t* buf, size_t n) { size_t k = 0; int c; while (k < n && (c = read()) >= 0) buf[k++] = (uint8_t)c; return k; }
  size_t readBytes(char* buf, size_t n) { return readBytes(reinterpret_cast<uint8_t*>(buf), n); }
  void setTimeout(unsigned long) {}
};

#define SERIAL_8N1 0x800001c
// A serial port a test can talk to. What the firmware writes is captured (the newest
// 1 MiB is kept); what a test feeds is what the firmware reads; a listener, when set, sees every byte the firmware
// writes as it is written, so a scripted device model can answer at once (host_feed from inside the listener).
class HardwareSerial : public Stream {
 public:
  HardwareSerial() {}
  explicit HardwareSerial(int /*uart*/) {}
  void begin(unsigned long baud, uint32_t = 0, int8_t = -1, int8_t = -1) { baud_ = baud; }
  void end() {}
  size_t setRxBufferSize(size_t n) { return n; }
  size_t write(uint8_t c) override {
    tx_.push_back(static_cast<char>(c));
    if (tx_.size() > kCaptureMax) tx_.erase(0, tx_.size() - kCaptureMax);
    if (listener_) listener_(c);
    return 1;
  }
  using Print::write;
  int available() override { return static_cast<int>(rx_.size() - rx_pos_); }
  int read() override { return rx_pos_ < rx_.size() ? static_cast<uint8_t>(rx_[rx_pos_++]) : -1; }
  int peek() override { return rx_pos_ < rx_.size() ? static_cast<uint8_t>(rx_[rx_pos_]) : -1; }
  explicit operator bool() const { return true; }

  // host-only: drive and observe the port from a test or a device model
  void host_feed(const uint8_t* bytes, size_t n) { rx_.append(reinterpret_cast<const char*>(bytes), n); }
  void host_feed(const char* text) { rx_.append(text); }
  const std::string& host_written() const { return tx_; }
  void host_clear() { tx_.clear(); rx_.clear(); rx_pos_ = 0; }
  void host_listen(std::function<void(uint8_t)> listener) { listener_ = std::move(listener); }
  unsigned long host_baud() const { return baud_; }

 private:
  static constexpr size_t kCaptureMax = 1u << 20;
  std::string tx_, rx_;
  size_t rx_pos_ = 0;
  unsigned long baud_ = 0;
  std::function<void(uint8_t)> listener_;
};
