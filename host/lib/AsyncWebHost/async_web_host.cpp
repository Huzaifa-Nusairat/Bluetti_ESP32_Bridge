#include "ESPAsyncWebServer.h"
namespace {
struct Route { std::string uri; WebRequestMethodComposite method; ArRequestHandlerFunction fn; };
struct Web { std::vector<Route> routes; std::vector<host_web_event> events; bool started = false; };
Web& web() { static Web w; return w; }
// What %NAME% placeholders become: ESPAsyncWebServer's template processing, enough to show the processor's output.
std::string expand_template(const char* content, const AwsTemplateProcessor& processor) {
  std::string in(content ? content : ""), out;
  if (!processor) return in;
  for (size_t i = 0; i < in.size();) {
    size_t a = in.find('%', i);
    if (a == std::string::npos) { out += in.substr(i); break; }
    size_t b = in.find('%', a + 1);
    if (b == std::string::npos) { out += in.substr(i); break; }
    out += in.substr(i, a - i);
    std::string name = in.substr(a + 1, b - a - 1);
    if (name.empty() || name.find_first_of(" \n\t;:{}") != std::string::npos) { out += '%'; i = a + 1; continue; }
    out += processor(String(name.c_str())).c_str();
    i = b + 1;
  }
  return out;
}
}
void AsyncWebServerRequest::send(int code, const char* type, const String& content) {
  code_ = code; type_ = type ? type : ""; body_ = content.c_str();
}
void AsyncWebServerRequest::send_P(int code, const char* type, const char* content, AwsTemplateProcessor processor) {
  code_ = code; type_ = type ? type : ""; body_ = expand_template(content, processor);
}
void AsyncEventSource::send(const char* message, const char* event, uint32_t id, uint32_t) {
  web().events.push_back({message ? message : "", event ? event : "", id});
}
void AsyncWebServer::on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction handler) {
  web().routes.push_back({uri ? uri : "", method, handler});
}
void AsyncWebServer::begin() { web().started = true; }
bool host_web_request(const char* uri, WebRequestMethodComposite method, int* code, std::string* body) {
  for (auto& r : web().routes) {
    if (r.uri == uri && (r.method & method)) {
      AsyncWebServerRequest req(uri, method);
      r.fn(&req);
      if (code) *code = req.host_code();
      if (body) *body = req.host_body();
      return true;
    }
  }
  return false;
}
bool host_web_started() { return web().started; }
const std::vector<host_web_event>& host_web_events() { return web().events; }
void host_web_reset() { web() = Web(); }
