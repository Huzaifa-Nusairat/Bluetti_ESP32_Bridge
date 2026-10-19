// Host stand-in for Arduino's String (the commonly used subset), over std::string for host tests.
#pragma once
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

class String {
 public:
  String() {}
  String(const char* s) : s_(s ? s : "") {}
  String(const std::string& s) : s_(s) {}
  String(char c) : s_(1, c) {}
  String(int v, unsigned char base = 10) { num(static_cast<long>(v), base); }
  String(unsigned int v, unsigned char base = 10) { unum(v, base); }
  String(long v, unsigned char base = 10) { num(v, base); }
  String(unsigned long v, unsigned char base = 10) { unum(v, base); }
  String(float v, unsigned int decimals = 2) { fmt(v, decimals); }
  String(double v, unsigned int decimals = 2) { fmt(v, decimals); }

  const char* c_str() const { return s_.c_str(); }
  unsigned int length() const { return static_cast<unsigned int>(s_.size()); }
  bool isEmpty() const { return s_.empty(); }
  char charAt(unsigned int i) const { return i < s_.size() ? s_[i] : 0; }
  void setCharAt(unsigned int i, char c) { if (i < s_.size()) s_[i] = c; }
  char operator[](unsigned int i) const { return charAt(i); }
  char& operator[](unsigned int i) { return s_[i]; }
  void reserve(unsigned int n) { s_.reserve(n); }

  int indexOf(char c, unsigned int from = 0) const { return pos(s_.find(c, from)); }
  int indexOf(const String& t, unsigned int from = 0) const { return pos(s_.find(t.s_, from)); }
  int lastIndexOf(char c) const { return pos(s_.rfind(c)); }
  int lastIndexOf(const String& t) const { return pos(s_.rfind(t.s_)); }
  String substring(unsigned int from) const { return from >= s_.size() ? String() : String(s_.substr(from)); }
  String substring(unsigned int from, unsigned int to) const {
    if (from > to) { unsigned int t = from; from = to; to = t; }
    if (from >= s_.size()) return String();
    return String(s_.substr(from, (to > s_.size() ? s_.size() : to) - from));
  }
  bool startsWith(const String& p) const { return s_.compare(0, p.s_.size(), p.s_) == 0; }
  bool endsWith(const String& p) const {
    return s_.size() >= p.s_.size() && s_.compare(s_.size() - p.s_.size(), p.s_.size(), p.s_) == 0;
  }
  bool equals(const String& o) const { return s_ == o.s_; }
  bool equalsIgnoreCase(const String& o) const {
    if (s_.size() != o.s_.size()) return false;
    for (size_t i = 0; i < s_.size(); i++) if (tolower((unsigned char)s_[i]) != tolower((unsigned char)o.s_[i])) return false;
    return true;
  }
  void trim() {
    size_t b = 0, e = s_.size();
    while (b < e && isspace((unsigned char)s_[b])) b++;
    while (e > b && isspace((unsigned char)s_[e - 1])) e--;
    s_ = s_.substr(b, e - b);
  }
  void toLowerCase() { for (auto& c : s_) c = (char)tolower((unsigned char)c); }
  void toUpperCase() { for (auto& c : s_) c = (char)toupper((unsigned char)c); }
  void replace(const String& from, const String& to) {
    if (from.s_.empty()) return;
    size_t p = 0;
    while ((p = s_.find(from.s_, p)) != std::string::npos) { s_.replace(p, from.s_.size(), to.s_); p += to.s_.size(); }
  }
  void replace(char from, char to) { for (auto& c : s_) if (c == from) c = to; }
  long toInt() const { return strtol(s_.c_str(), nullptr, 10); }
  float toFloat() const { return strtof(s_.c_str(), nullptr); }
  double toDouble() const { return strtod(s_.c_str(), nullptr); }
  bool concat(const String& o) { s_ += o.s_; return true; }

  String& operator+=(const String& o) { s_ += o.s_; return *this; }
  String& operator+=(const char* o) { s_ += (o ? o : ""); return *this; }
  String& operator+=(char c) { s_ += c; return *this; }
  String& operator+=(int v) { return *this += String(v); }
  String& operator+=(unsigned int v) { return *this += String(v); }
  String& operator+=(long v) { return *this += String(v); }
  String& operator+=(unsigned long v) { return *this += String(v); }
  String& operator+=(float v) { return *this += String(v); }
  String& operator+=(double v) { return *this += String(v); }
  friend String operator+(const String& a, const String& b) { return String(a.s_ + b.s_); }
  friend String operator+(const String& a, const char* b) { return String(a.s_ + (b ? b : "")); }
  friend String operator+(const char* a, const String& b) { return String(std::string(a ? a : "") + b.s_); }
  friend String operator+(const String& a, char b) { return String(a.s_ + b); }
  friend String operator+(const String& a, int b) { return a + String(b); }
  friend String operator+(const String& a, unsigned int b) { return a + String(b); }
  friend String operator+(const String& a, long b) { return a + String(b); }
  friend String operator+(const String& a, unsigned long b) { return a + String(b); }
  friend String operator+(const String& a, float b) { return a + String(b); }
  friend String operator+(const String& a, double b) { return a + String(b); }
  bool operator==(const String& o) const { return s_ == o.s_; }
  bool operator==(const char* o) const { return s_ == (o ? o : ""); }
  bool operator!=(const String& o) const { return s_ != o.s_; }
  bool operator!=(const char* o) const { return !(*this == o); }
  bool operator<(const String& o) const { return s_ < o.s_; }

 private:
  static int pos(size_t p) { return p == std::string::npos ? -1 : static_cast<int>(p); }
  void num(long v, unsigned char base) {
    if (v < 0 && base == 10) { unum(static_cast<unsigned long>(-v), base); s_ = "-" + s_; } else unum(static_cast<unsigned long>(v), base);
  }
  void unum(unsigned long v, unsigned char base) {
    if (base < 2) base = 10;
    char buf[70]; int i = 69; buf[i] = 0;
    do { int d = static_cast<int>(v % base); buf[--i] = static_cast<char>(d < 10 ? '0' + d : 'A' + d - 10); v /= base; } while (v);
    s_ = &buf[i];
  }
  void fmt(double v, unsigned int decimals) { char buf[64]; snprintf(buf, sizeof buf, "%.*f", (int)decimals, v); s_ = buf; }
  std::string s_;
};
