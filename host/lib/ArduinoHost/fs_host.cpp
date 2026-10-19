#include "LittleFS.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ftw.h>

namespace fs {

File::File(const std::string& host, const std::string& name, const char* mode) {
  struct stat sb;
  auto st = std::make_shared<State>();
  st->host = host; st->name = name;
  if (stat(host.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
    st->dir = opendir(host.c_str());
  } else {
    std::string m = mode && *mode ? mode : "r";
    if (m == "w") m = "w+"; else if (m == "a") m = "a+"; else m = "r";
    st->fp = fopen(host.c_str(), (m + "b").c_str());
  }
  if (st->fp || st->dir) st_ = st;
}

int File::available() {
  if (!st_ || !st_->fp) return 0;
  long here = ftell(st_->fp); fseek(st_->fp, 0, SEEK_END); long end = ftell(st_->fp); fseek(st_->fp, here, SEEK_SET);
  return (int)(end - here);
}

int File::peek() {
  if (!st_ || !st_->fp) return -1;
  int c = fgetc(st_->fp); if (c >= 0) ungetc(c, st_->fp); return c;
}

size_t File::size() const {
  struct stat sb; return st_ && stat(st_->host.c_str(), &sb) == 0 ? (size_t)sb.st_size : 0;
}

const char* File::name() const {
  if (!st_) return "";
  const char* slash = strrchr(st_->name.c_str(), '/');
  return slash ? slash + 1 : st_->name.c_str();
}

File File::openNextFile(const char* mode) {
  if (!st_ || !st_->dir) return File();
  struct dirent* e;
  while ((e = readdir(st_->dir)) != nullptr) {
    if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")) continue;
    std::string name = st_->name == "/" ? "/" + std::string(e->d_name) : st_->name + "/" + e->d_name;
    return File(st_->host + "/" + e->d_name, name, mode);
  }
  return File();
}

std::string FS::host(const char* path) {
  if (root_.empty()) {
    const char* env = getenv(root_env_);
    if (env && *env) {
      root_ = env;
    } else {
      char tmpl[] = "/tmp/host-littlefs-XXXXXX";
      root_ = mkdtemp(tmpl) ? tmpl : "/tmp";
    }
    ::mkdir(root_.c_str(), 0700);
  }
  std::string p = path ? path : "/";
  if (p.empty() || p[0] != '/') p = "/" + p;
  return root_ + p;
}

File FS::open(const char* path, const char* mode, bool) {
  std::string m = mode ? mode : "r";
  struct stat sb;
  std::string h = host(path);
  if (m == "r" && stat(h.c_str(), &sb) != 0) return File();
  return File(h, path ? path : "/", mode);
}

bool FS::exists(const char* path) { struct stat sb; return stat(host(path).c_str(), &sb) == 0; }
bool FS::remove(const char* path) { return ::unlink(host(path).c_str()) == 0; }
bool FS::rename(const char* from, const char* to) { return ::rename(host(from).c_str(), host(to).c_str()) == 0; }
bool FS::mkdir(const char* path) { return ::mkdir(host(path).c_str(), 0700) == 0 || exists(path); }
bool FS::rmdir(const char* path) { return ::rmdir(host(path).c_str()) == 0; }

}  // namespace fs

LittleFSClass LittleFS;

bool LittleFSClass::begin(bool, const char*, uint8_t, const char*) { host("/"); return true; }

static int rm_entry(const char* p, const struct stat*, int, struct FTW* f) { return f->level == 0 ? 0 : ::remove(p); }
bool LittleFSClass::format() { nftw(host("/").c_str(), rm_entry, 16, FTW_DEPTH | FTW_PHYS); return true; }

static size_t used_total;
static int add_size(const char*, const struct stat* sb, int flag, struct FTW*) {
  if (flag == FTW_F) used_total += (size_t)sb->st_size;
  return 0;
}
size_t LittleFSClass::usedBytes() { used_total = 0; nftw(host("/").c_str(), add_size, 16, FTW_PHYS); return used_total; }

#include "WiFi.h"
WiFiClass WiFi;

#include "ESPmDNS.h"
#include "esp_task_wdt.h"
MDNSResponder MDNS;
void configTime(long, int, const char*, const char*, const char*) {}
bool getLocalTime(struct tm* info, uint32_t) { time_t now = time(nullptr); localtime_r(&now, info); return false; }
esp_reset_reason_t esp_reset_reason(void) { return ESP_RST_POWERON; }
void esp_restart(void) {}
uint32_t esp_get_free_heap_size(void) { return 200000; }
esp_err_t esp_task_wdt_init(const esp_task_wdt_config_t*) { return ESP_OK; }
esp_err_t esp_task_wdt_reconfigure(const esp_task_wdt_config_t*) { return ESP_OK; }
esp_err_t esp_task_wdt_add(void*) { return ESP_OK; }
esp_err_t esp_task_wdt_reset(void) { return ESP_OK; }
esp_err_t esp_task_wdt_status(void*) { return ESP_OK; }
