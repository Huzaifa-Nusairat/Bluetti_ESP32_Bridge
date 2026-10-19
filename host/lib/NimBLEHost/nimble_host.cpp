#include "NimBLEDevice.h"
#include <map>
#include <memory>
namespace {
struct Peer {
  std::vector<std::unique_ptr<NimBLEAdvertisedDevice>> pending;
  bool service = true, connect_ok = true;
  notify_callback notify;
  NimBLERemoteCharacteristic* notify_char = nullptr;
  std::vector<std::vector<uint8_t>> written;
  NimBLEScan scan;
  std::vector<std::unique_ptr<NimBLEClient>> clients;
  std::map<std::string, std::unique_ptr<NimBLERemoteCharacteristic>> chars;
  NimBLERemoteService remote;
  NimBLEClient* current = nullptr;
  NimBLEClientCallbacks* current_cb = nullptr;
  int scans = 0;
};
Peer& peer() { static Peer p; return p; }
}
bool NimBLERemoteCharacteristic::registerForNotify(notify_callback cb, bool, bool) {
  peer().notify = cb; peer().notify_char = this; return true;
}
bool NimBLERemoteCharacteristic::writeValue(const uint8_t* data, size_t length, bool) {
  peer().written.emplace_back(data, data + length); return true;
}
NimBLERemoteCharacteristic* NimBLERemoteService::getCharacteristic(const NimBLEUUID& u) {
  if (!peer().service) return nullptr;
  auto& slot = peer().chars[u.toString()];
  if (!slot) slot.reset(new NimBLERemoteCharacteristic(u));
  return slot.get();
}
bool NimBLEClient::connect(NimBLEAdvertisedDevice*, bool) {
  connected_ = peer().connect_ok;
  if (connected_) { peer().current = this; peer().current_cb = cb_; if (cb_) cb_->onConnect(this); }
  return connected_;
}
NimBLERemoteService* NimBLEClient::getService(const NimBLEUUID&) {
  return connected_ && peer().service ? &peer().remote : nullptr;
}
int NimBLEClient::disconnect(uint8_t) {
  if (connected_) { connected_ = false; if (cb_) cb_->onDisconnect(this); }
  return 0;
}
bool NimBLEScan::start(uint32_t, bool) {
  ++peer().scans; running_ = true;
  auto pending = std::move(peer().pending);
  peer().pending.clear();
  for (auto& d : pending) {
    NimBLEAdvertisedDevice* raw = d.release();  // the firmware keeps the pointer, as with the real stack
    if (cb_) cb_->onResult(raw);
  }
  return true;
}
NimBLEScan* NimBLEDevice::getScan() { return &peer().scan; }
NimBLEClient* NimBLEDevice::createClient() {
  peer().clients.emplace_back(new NimBLEClient());
  return peer().clients.back().get();
}
void host_ble_advertise(const std::string& name, const std::string& service_uuid, const std::string& address) {
  peer().pending.emplace_back(new NimBLEAdvertisedDevice(name, NimBLEUUID(service_uuid), address));
}
void host_ble_set_service(bool present) { peer().service = present; }
void host_ble_set_connect(bool ok) { peer().connect_ok = ok; }
bool host_ble_notify(const uint8_t* data, size_t len) {
  if (!peer().notify) return false;
  std::vector<uint8_t> copy(data, data + len);
  peer().notify(peer().notify_char, copy.data(), copy.size(), true);
  return true;
}
const std::vector<std::vector<uint8_t>>& host_ble_written() { return peer().written; }
void host_ble_drop_link() { if (peer().current) peer().current->disconnect(); }
void host_ble_reset() {
  Peer& p = peer();
  p.pending.clear(); p.service = true; p.connect_ok = true; p.notify = nullptr; p.notify_char = nullptr;
  p.written.clear(); p.current = nullptr; p.current_cb = nullptr; p.scans = 0; p.scan.stop();
}
int host_ble_scans() { return peer().scans; }
