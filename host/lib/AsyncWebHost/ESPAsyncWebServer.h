// Host stand-in for ESPAsyncWebServer 3.x (the calls this firmware makes). Nothing listens: on() records each route,
// and a test calls a route as if a browser had asked (host_web_request) and reads the answer; events.send()
// records each server-sent event (host_web_events).
#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Arduino.h"

typedef enum { HTTP_GET = 0b00000001, HTTP_POST = 0b00000010, HTTP_DELETE = 0b00000100, HTTP_PUT = 0b00001000,
               HTTP_PATCH = 0b00010000, HTTP_HEAD = 0b00100000, HTTP_OPTIONS = 0b01000000, HTTP_ANY = 0b01111111 }
    WebRequestMethod;
typedef uint8_t WebRequestMethodComposite;
typedef std::function<String(const String&)> AwsTemplateProcessor;

class AsyncWebServerRequest {
 public:
  AsyncWebServerRequest(const std::string& url, WebRequestMethodComposite method) : url_(url), method_(method) {}
  void send(int code, const char* type = "", const String& content = String());
  void send(int code, const String& type, const String& content = String()) { send(code, type.c_str(), content); }
  void send_P(int code, const char* type, const char* content, AwsTemplateProcessor processor = nullptr);
  void redirect(const char* url) { code_ = 302; body_ = url ? url : ""; }
  String url() const { return String(url_.c_str()); }
  WebRequestMethodComposite method() const { return method_; }
  bool hasParam(const char*, bool = false) const { return false; }
  int host_code() const { return code_; }
  std::string host_type() const { return type_; }
  std::string host_body() const { return body_; }
 private:
  std::string url_, type_, body_;
  WebRequestMethodComposite method_;
  int code_ = 0;
};

typedef std::function<void(AsyncWebServerRequest*)> ArRequestHandlerFunction;

class AsyncEventSourceClient {
 public:
  uint32_t lastId() const { return 0; }
  void send(const char*, const char* = nullptr, uint32_t = 0, uint32_t = 0) {}
};
typedef std::function<void(AsyncEventSourceClient*)> ArEventHandlerFunction;

class AsyncWebHandler {
 public:
  virtual ~AsyncWebHandler() {}
};

class AsyncEventSource : public AsyncWebHandler {
 public:
  explicit AsyncEventSource(const char* url) : url_(url ? url : "") {}
  void onConnect(ArEventHandlerFunction cb) { connect_ = cb; }
  void send(const char* message, const char* event = nullptr, uint32_t id = 0, uint32_t reconnect = 0);
  size_t count() const { return 0; }
 private:
  std::string url_;
  ArEventHandlerFunction connect_;
};

class AsyncWebServer {
 public:
  explicit AsyncWebServer(uint16_t port) : port_(port) {}
  void on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler);
  void on(const char* uri, ArRequestHandlerFunction handler) { on(uri, HTTP_ANY, handler); }
  AsyncWebHandler& addHandler(AsyncWebHandler* handler) { return *handler; }
  void onNotFound(ArRequestHandlerFunction handler) { not_found_ = handler; }
  void begin();
  void end() {}
  uint16_t host_port() const { return port_; }
 private:
  uint16_t port_;
  ArRequestHandlerFunction not_found_;
};

struct host_web_event { std::string message, event; uint32_t id; };

// Host-only controls.
bool host_web_request(const char* uri, WebRequestMethodComposite method, int* code, std::string* body);
bool host_web_started();
const std::vector<host_web_event>& host_web_events();
void host_web_reset();
