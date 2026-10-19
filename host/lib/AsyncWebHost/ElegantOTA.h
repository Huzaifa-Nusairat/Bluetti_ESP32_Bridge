// Host stand-in for ElegantOTA 3.x: begin() is recorded (with or without credentials); no update can happen.
#pragma once
#include <string>
#include "ESPAsyncWebServer.h"
class ElegantOTAClass {
 public:
  void begin(AsyncWebServer* server, const char* user = "", const char* pass = "") {
    server_ = server; user_ = user ? user : ""; has_pass_ = pass && *pass;
  }
  void loop() {}
  void setAutoReboot(bool) {}
  bool host_started() const { return server_ != nullptr; }
  std::string host_user() const { return user_; }
  bool host_has_password() const { return has_pass_; }
 private:
  AsyncWebServer* server_ = nullptr;
  std::string user_;
  bool has_pass_ = false;
};
extern ElegantOTAClass ElegantOTA;
