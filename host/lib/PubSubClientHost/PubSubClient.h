// Host stand-in for knolleary/PubSubClient 2.8 (the calls this firmware makes). No network: a test decides whether
// connect() succeeds (host_mqtt_set_connect), reads what was published and subscribed, and delivers a message to
// the firmware's callback (host_mqtt_deliver).
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Arduino.h"
#include "WiFi.h"

#define MQTT_MAX_PACKET_SIZE 256
#define MQTT_CONNECTED 0
#define MQTT_DISCONNECTED (-1)
#define MQTT_CONNECT_FAILED (-2)
#define MQTT_CALLBACK_SIGNATURE std::function<void(char*, uint8_t*, unsigned int)> callback

struct host_mqtt_message { std::string topic, payload; bool retained; };

class PubSubClient {
 public:
  PubSubClient() {}
  explicit PubSubClient(WiFiClient& client) { (void)client; }
  PubSubClient& setServer(const char* host, uint16_t port) { host_ = host ? host : ""; port_ = port; return *this; }
  PubSubClient& setServer(IPAddress, uint16_t port) { port_ = port; return *this; }
  PubSubClient& setCallback(MQTT_CALLBACK_SIGNATURE);
  PubSubClient& setClient(WiFiClient&) { return *this; }
  bool setBufferSize(uint16_t size) { buffer_ = size; return true; }
  uint16_t getBufferSize() { return buffer_; }
  PubSubClient& setKeepAlive(uint16_t) { return *this; }
  PubSubClient& setSocketTimeout(uint16_t) { return *this; }
  bool connect(const char* id);
  bool connect(const char* id, const char* user, const char* pass);
  bool connect(const char* id, const char* user, const char* pass, const char*, uint8_t, bool, const char*);
  void disconnect();
  bool publish(const char* topic, const char* payload);
  bool publish(const char* topic, const char* payload, bool retained);
  bool publish(const char* topic, const uint8_t* payload, unsigned int length);
  bool publish(const char* topic, const uint8_t* payload, unsigned int length, bool retained);
  bool subscribe(const char* topic, uint8_t qos = 0);
  bool unsubscribe(const char* topic);
  bool loop() { return connected_; }
  bool connected() { return connected_; }
  int state() { return connected_ ? MQTT_CONNECTED : state_; }
  std::string host_host() const { return host_; }
  uint16_t host_port() const { return port_; }
 private:
  std::string host_;
  uint16_t port_ = 0, buffer_ = MQTT_MAX_PACKET_SIZE;
  bool connected_ = false;
  int state_ = MQTT_DISCONNECTED;
};

// Host-only controls.
void host_mqtt_set_connect(bool ok);                    // default true
void host_mqtt_set_publish(bool ok);                    // default true: publish() succeeds while connected
bool host_mqtt_deliver(const char* topic, const char* payload);  // false when no callback is set
const std::vector<host_mqtt_message>& host_mqtt_published();
const std::vector<std::string>& host_mqtt_subscribed();
std::string host_mqtt_last_client_id();
std::string host_mqtt_last_user();
void host_mqtt_reset();
