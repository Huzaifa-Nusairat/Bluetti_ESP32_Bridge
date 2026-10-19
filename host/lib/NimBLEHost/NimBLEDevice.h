// Host stand-in for NimBLE-Arduino 1.4 (client side only, the calls this firmware makes). Nothing touches a radio.
// A test scripts the peer: host_ble_advertise() makes the next scan report a device, host_ble_set_service()
// decides whether the connected peer has the service and its two characteristics, host_ble_notify() delivers a
// notification to the registered callback, host_ble_written() returns every writeValue() in order.
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Arduino.h"
#include "freertos_queue.h"

class NimBLEUUID {
 public:
  NimBLEUUID() {}
  NimBLEUUID(const char* s) : s_(s ? s : "") {}
  NimBLEUUID(const std::string& s) : s_(s) {}
  std::string toString() const { return s_; }
  bool operator==(const NimBLEUUID& o) const { return s_ == o.s_; }
 private:
  std::string s_;
};

class NimBLEAddress {
 public:
  explicit NimBLEAddress(const std::string& s = "00:00:00:00:00:00") : s_(s) {}
  std::string toString() const { return s_; }
 private:
  std::string s_;
};

class NimBLEAdvertisedDevice {
 public:
  NimBLEAdvertisedDevice(const std::string& name, const NimBLEUUID& service, const std::string& address)
      : name_(name), service_(service), address_(address) {}
  std::string getName() { return name_; }
  NimBLEAddress getAddress() { return address_; }
  bool haveServiceUUID() { return !service_.toString().empty(); }
  bool isAdvertisingService(const NimBLEUUID& u) { return service_ == u; }
  std::string toString() { return "Name: " + name_ + ", Address: " + address_.toString(); }
 private:
  std::string name_;
  NimBLEUUID service_;
  NimBLEAddress address_;
};

class NimBLERemoteCharacteristic;
typedef std::function<void(NimBLERemoteCharacteristic*, uint8_t*, size_t, bool)> notify_callback;

class NimBLERemoteCharacteristic {
 public:
  explicit NimBLERemoteCharacteristic(const NimBLEUUID& u) : uuid_(u) {}
  bool canRead() { return true; }
  bool canNotify() { return true; }
  std::string readValue() { return std::string(); }
  bool registerForNotify(notify_callback cb, bool = true, bool = true);
  bool writeValue(const uint8_t* data, size_t length, bool response = false);
  NimBLEUUID getUUID() { return uuid_; }
 private:
  NimBLEUUID uuid_;
};

class NimBLERemoteService {
 public:
  NimBLERemoteCharacteristic* getCharacteristic(const NimBLEUUID& u);
};

class NimBLEClient;
class NimBLEClientCallbacks {
 public:
  virtual ~NimBLEClientCallbacks() {}
  virtual void onConnect(NimBLEClient*) {}
  virtual void onDisconnect(NimBLEClient*) {}
};

class NimBLEClient {
 public:
  void setClientCallbacks(NimBLEClientCallbacks* cb, bool = true) { cb_ = cb; }
  bool connect(NimBLEAdvertisedDevice* device, bool = true);
  NimBLERemoteService* getService(const NimBLEUUID& u);
  int disconnect(uint8_t = 0x13);
  bool isConnected() { return connected_; }
 private:
  NimBLEClientCallbacks* cb_ = nullptr;
  bool connected_ = false;
};

class NimBLEAdvertisedDeviceCallbacks {
 public:
  virtual ~NimBLEAdvertisedDeviceCallbacks() {}
  virtual void onResult(NimBLEAdvertisedDevice*) {}
};

class NimBLEScan {
 public:
  void setAdvertisedDeviceCallbacks(NimBLEAdvertisedDeviceCallbacks* cb, bool = false) { cb_ = cb; }
  void setInterval(uint16_t) {}
  void setWindow(uint16_t) {}
  void setActiveScan(bool) {}
  bool start(uint32_t duration, bool is_continue = false);
  bool stop() { running_ = false; return true; }
  bool isScanning() { return running_; }
 private:
  NimBLEAdvertisedDeviceCallbacks* cb_ = nullptr;
  bool running_ = false;
};

class NimBLEDevice {
 public:
  static void init(const std::string&) {}
  static NimBLEScan* getScan();
  static NimBLEClient* createClient();
  static bool setMTU(uint16_t) { return true; }
  static void deinit(bool = false) {}
};

// NimBLE-Arduino 1.x maps the Bluedroid names onto its own classes.
#define BLEDevice NimBLEDevice
#define BLEScan NimBLEScan
#define BLEClient NimBLEClient
#define BLEClientCallbacks NimBLEClientCallbacks
#define BLEAdvertisedDevice NimBLEAdvertisedDevice
#define BLEAdvertisedDeviceCallbacks NimBLEAdvertisedDeviceCallbacks
#define BLERemoteService NimBLERemoteService
#define BLERemoteCharacteristic NimBLERemoteCharacteristic
#define BLEUUID NimBLEUUID
#define BLEAddress NimBLEAddress

// Host-only controls.
void host_ble_advertise(const std::string& name, const std::string& service_uuid,
                           const std::string& address = "aa:bb:cc:dd:ee:ff");
void host_ble_set_service(bool present);              // default true: service and both characteristics exist
void host_ble_set_connect(bool ok);                   // default true
bool host_ble_notify(const uint8_t* data, size_t len);  // false when nothing registered for notifications
const std::vector<std::vector<uint8_t>>& host_ble_written();
void host_ble_drop_link();                            // the peer goes away: onDisconnect is called
void host_ble_reset();
int host_ble_scans();                                 // how many times start() was called
