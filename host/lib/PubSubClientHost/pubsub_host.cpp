#include "PubSubClient.h"
#include <string.h>
namespace {
struct Broker {
  bool connect_ok = true, publish_ok = true;
  std::function<void(char*, uint8_t*, unsigned int)> cb;
  std::vector<host_mqtt_message> published;
  std::vector<std::string> subscribed;
  std::string client_id, user;
};
Broker& broker() { static Broker b; return b; }
}
PubSubClient& PubSubClient::setCallback(MQTT_CALLBACK_SIGNATURE) { broker().cb = callback; return *this; }
bool PubSubClient::connect(const char* id) { return connect(id, nullptr, nullptr); }
bool PubSubClient::connect(const char* id, const char* user, const char* pass) {
  (void)pass;
  broker().client_id = id ? id : ""; broker().user = user ? user : "";
  connected_ = broker().connect_ok;
  state_ = connected_ ? MQTT_CONNECTED : MQTT_CONNECT_FAILED;
  return connected_;
}
bool PubSubClient::connect(const char* id, const char* user, const char* pass, const char*, uint8_t, bool, const char*) {
  return connect(id, user, pass);
}
void PubSubClient::disconnect() { connected_ = false; state_ = MQTT_DISCONNECTED; }
bool PubSubClient::publish(const char* topic, const char* payload) { return publish(topic, payload, false); }
bool PubSubClient::publish(const char* topic, const char* payload, bool retained) {
  return publish(topic, reinterpret_cast<const uint8_t*>(payload), payload ? (unsigned)strlen(payload) : 0u, retained);
}
bool PubSubClient::publish(const char* topic, const uint8_t* payload, unsigned int length) {
  return publish(topic, payload, length, false);
}
bool PubSubClient::publish(const char* topic, const uint8_t* payload, unsigned int length, bool retained) {
  if (!connected_ || !broker().publish_ok) return false;
  broker().published.push_back({topic ? topic : "", std::string(reinterpret_cast<const char*>(payload), length), retained});
  return true;
}
bool PubSubClient::subscribe(const char* topic, uint8_t) {
  if (!connected_) return false;
  broker().subscribed.emplace_back(topic ? topic : "");
  return true;
}
bool PubSubClient::unsubscribe(const char*) { return connected_; }
void host_mqtt_set_connect(bool ok) { broker().connect_ok = ok; }
void host_mqtt_set_publish(bool ok) { broker().publish_ok = ok; }
bool host_mqtt_deliver(const char* topic, const char* payload) {
  if (!broker().cb) return false;
  std::string t(topic ? topic : "");
  std::vector<uint8_t> p(payload, payload + strlen(payload));
  p.push_back(0);  // the firmware writes payload[length] = '\0', as PubSubClient's buffer allows
  broker().cb(&t[0], p.data(), (unsigned)(p.size() - 1));
  return true;
}
const std::vector<host_mqtt_message>& host_mqtt_published() { return broker().published; }
const std::vector<std::string>& host_mqtt_subscribed() { return broker().subscribed; }
std::string host_mqtt_last_client_id() { return broker().client_id; }
std::string host_mqtt_last_user() { return broker().user; }
void host_mqtt_reset() { Broker& b = broker(); b = Broker(); }
