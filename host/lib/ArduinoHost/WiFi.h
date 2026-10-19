// Host stand-in for the ESP32 WiFi API: nothing connects; enough for code that only refers to it.
#pragma once
#include "Arduino.h"

#include <time.h>
#define WL_CONNECTED 3
#define WL_DISCONNECTED 6
typedef int wl_status_t;
typedef int wifi_mode_t;
#define WIFI_OFF 0
#define WIFI_STA 1
#define WIFI_AP 2
#define WIFI_AP_STA 3
#define WIFI_AUTH_OPEN 0
#define WIFI_SCAN_RUNNING (-1)
#define WIFI_SCAN_FAILED (-2)


class IPAddress : public Printable {
 public:
  IPAddress(uint8_t a = 0, uint8_t b = 0, uint8_t c = 0, uint8_t d = 0) { o_[0] = a; o_[1] = b; o_[2] = c; o_[3] = d; }
  String toString() const { char b[16]; snprintf(b, sizeof b, "%u.%u.%u.%u", o_[0], o_[1], o_[2], o_[3]); return String(b); }
  uint8_t operator[](int i) const { return o_[i]; }
  size_t printTo(Print& p) const override { return p.print(toString()); }
 private:
  uint8_t o_[4];
};

class WiFiClient : public Stream {
 public:
  size_t write(uint8_t) override { return 1; }
  using Print::write;
  bool connected() { return false; }
  void stop() {}
  explicit operator bool() const { return false; }
  IPAddress remoteIP() const { return IPAddress(); }
};

class WiFiServer {
 public:
  explicit WiFiServer(uint16_t = 23) {}
  void begin() {}
  void setNoDelay(bool) {}
  bool hasClient() { return false; }
  WiFiClient available() { return WiFiClient(); }
  WiFiClient accept() { return WiFiClient(); }
};

class WiFiClass {
 public:
  wl_status_t status() { return WL_DISCONNECTED; }
  String macAddress() { return String("00:00:00:00:00:00"); }
  void macAddress(uint8_t* mac) { for (int i = 0; i < 6; i++) mac[i] = 0; }
  IPAddress localIP() { return IPAddress(); }
  IPAddress softAPIP() { return IPAddress(192, 168, 4, 1); }
  bool begin(const char* = nullptr, const char* = nullptr) { return false; }
  bool disconnect(bool = false) { return true; }
  bool reconnect() { return true; }
  bool mode(int) { return true; }
  int getMode() { return WIFI_OFF; }
  bool softAP(const char* = nullptr, const char* = nullptr, int = 1, int = 0, int = 4) { return true; }
  bool softAPdisconnect(bool = false) { return true; }
  int16_t scanNetworks(bool = false, bool = false) { return 0; }
  int16_t scanComplete() { return 0; }
  void scanDelete() {}
  int32_t channel(uint8_t = 0) { return 0; }
  int encryptionType(uint8_t) { return WIFI_AUTH_OPEN; }
  String SSID(uint8_t) { return String(); }
  int32_t RSSI(uint8_t) { return 0; }
  bool setHostname(const char*) { return true; }
  bool setAutoReconnect(bool) { return true; }
  int RSSI() { return 0; }
  String SSID() { return String(); }
};
extern WiFiClass WiFi;
