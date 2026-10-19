// Host stand-in for Adafruit GFX 1.11: a 1-bit frame buffer the firmware draws into, so a test can read pixels
// (host_pixel) and the text printed (host_text). Fonts are not rendered: print() only records the text.
#pragma once
#include <string>
#include <vector>
#include "Arduino.h"

class Adafruit_GFX : public Print {
 public:
  Adafruit_GFX(int16_t w, int16_t h) : w_(w), h_(h), px_((size_t)w * h, 0) {}
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x >= 0 && y >= 0 && x < w_ && y < h_) px_[(size_t)y * w_ + x] = color ? 1 : 0;
  }
  void writePixel(int16_t x, int16_t y, uint16_t color) { drawPixel(x, y, color); }
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for (int16_t j = y; j < y + h; ++j) for (int16_t i = x; i < x + w; ++i) drawPixel(i, j, color);
  }
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color); drawFastHLine(x, y + h - 1, w, color);
    drawFastVLine(x, y, h, color); drawFastVLine(x + w - 1, y, h, color);
  }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) { for (int16_t i = 0; i < w; ++i) drawPixel(x + i, y, color); }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) { for (int16_t j = 0; j < h; ++j) drawPixel(x, y + j, color); }
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void fillScreen(uint16_t color) { fillRect(0, 0, w_, h_, color); }
  void drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color);
  void setCursor(int16_t x, int16_t y) { cx_ = x; cy_ = y; }
  void setTextSize(uint8_t s) { size_ = s; }
  void setTextColor(uint16_t c) { fg_ = c; }
  void setTextColor(uint16_t c, uint16_t bg) { fg_ = c; bg_ = bg; }
  void setTextWrap(bool) {}
  void setRotation(uint8_t) {}
  int16_t width() const { return w_; }
  int16_t height() const { return h_; }
  int16_t getCursorX() const { return cx_; }
  int16_t getCursorY() const { return cy_; }
  size_t write(uint8_t c) override { text_ += static_cast<char>(c); return 1; }
  using Print::write;
  // Host-only.
  int host_pixel(int16_t x, int16_t y) const { return (x >= 0 && y >= 0 && x < w_ && y < h_) ? px_[(size_t)y * w_ + x] : -1; }
  const std::string& host_text() const { return text_; }
  void host_clear_text() { text_.clear(); }
 protected:
  int16_t w_, h_;
  std::vector<uint8_t> px_;
  std::string text_;
  int16_t cx_ = 0, cy_ = 0;
  uint8_t size_ = 1;
  uint16_t fg_ = 1, bg_ = 0;
};
