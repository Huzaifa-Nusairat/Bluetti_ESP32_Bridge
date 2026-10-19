#include "WiFiManager.h"
#include <map>
namespace {
struct Portal { bool connect_ok = true, portal = false; int resets = 0; std::map<std::string, std::string> typed; };
Portal& portal() { static Portal p; return p; }
}
void WiFiManager::resetSettings() { ++portal().resets; }
bool WiFiManager::autoConnect(const char* ap_name, const char*) {
  ssid_ = ap_name ? ap_name : "";
  if (portal().portal) {
    if (ap_) ap_(this);
    for (auto* p : params_) {
      auto it = portal().typed.find(p->getID());
      if (it != portal().typed.end()) p->setValue(it->second.c_str(), p->getValueLength());
    }
    if (portal().connect_ok && save_) save_();
  }
  return portal().connect_ok;
}
void host_wifimanager_set_connect(bool ok) { portal().connect_ok = ok; }
void host_wifimanager_set_portal(bool opens) { portal().portal = opens; }
void host_wifimanager_set_value(const char* id, const char* value) { portal().typed[id ? id : ""] = value ? value : ""; }
int host_wifimanager_resets() { return portal().resets; }
void host_wifimanager_reset() { portal() = Portal(); }
