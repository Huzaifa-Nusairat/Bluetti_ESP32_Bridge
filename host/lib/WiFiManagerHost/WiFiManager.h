// Host stand-in for tzapu/WiFiManager 2.0 (the calls this firmware makes). No portal opens; autoConnect() returns
// what the test chose (host_wifimanager_set_connect), and a test can fill the parameters as if typed in the portal.
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Arduino.h"
#include "WiFi.h"

class WiFiManagerParameter {
 public:
  WiFiManagerParameter(const char* id, const char* label, const char* def, int length, const char* custom = "")
      : id_(id ? id : ""), label_(label ? label : ""), value_(def ? def : ""), length_(length) { (void)custom; }
  const char* getID() const { return id_.c_str(); }
  const char* getValue() const { return value_.c_str(); }
  int getValueLength() const { return length_; }
  void setValue(const char* v, int length) { value_ = v ? std::string(v).substr(0, (size_t)length) : ""; length_ = length; }
 private:
  std::string id_, label_, value_;
  int length_;
};

class WiFiManager {
 public:
  void resetSettings();
  void setConfigPortalTimeout(unsigned long seconds) { timeout_ = seconds; }
  void setConnectTimeout(unsigned long) {}
  void setSaveConfigCallback(std::function<void()> cb) { save_ = cb; }
  void setAPCallback(std::function<void(WiFiManager*)> cb) { ap_ = cb; }
  bool addParameter(WiFiManagerParameter* p) { params_.push_back(p); return true; }
  String getConfigPortalSSID() { return String(ssid_.c_str()); }
  bool autoConnect(const char* ap_name = "ESP", const char* ap_password = nullptr);
  bool startConfigPortal(const char* ap_name = "ESP", const char* ap_password = nullptr) {
    return autoConnect(ap_name, ap_password);
  }
  unsigned long host_portal_timeout() const { return timeout_; }
 private:
  std::string ssid_;
  unsigned long timeout_ = 0;
  std::function<void()> save_;
  std::function<void(WiFiManager*)> ap_;
  std::vector<WiFiManagerParameter*> params_;
};

// Host-only controls.
void host_wifimanager_set_connect(bool ok);         // default true
void host_wifimanager_set_portal(bool opens);       // default false: saved credentials work, no portal, no save
void host_wifimanager_set_value(const char* id, const char* value);  // what the user "types" when the portal opens
int host_wifimanager_resets();
void host_wifimanager_reset();
