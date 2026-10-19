// Host stand-in for Adafruit SSD1306 2.5: the GFX frame buffer plus begin()/display() bookkeeping; no I2C traffic.
// host_frames() counts display() calls (what reached the panel on the board).
#pragma once
#include "Adafruit_GFX.h"
#include "Wire.h"
#define SSD1306_BLACK 0
#define SSD1306_WHITE 1
#define SSD1306_INVERSE 2
#define BLACK SSD1306_BLACK
#define WHITE SSD1306_WHITE
#define INVERSE SSD1306_INVERSE
#define SSD1306_EXTERNALVCC 0x01
#define SSD1306_SWITCHCAPVCC 0x02
class Adafruit_SSD1306 : public Adafruit_GFX {
 public:
  Adafruit_SSD1306(uint8_t w, uint8_t h, TwoWire* wire = &Wire, int8_t rst = -1, uint32_t = 400000UL, uint32_t = 100000UL)
      : Adafruit_GFX(w, h) { (void)wire; (void)rst; }
  bool begin(uint8_t vcc = SSD1306_SWITCHCAPVCC, uint8_t addr = 0, bool = true, bool = true) {
    vcc_ = vcc; addr_ = addr; begun_ = true; return true;
  }
  void display() { ++frames_; }
  void clearDisplay() { fillScreen(0); text_.clear(); }
  void invertDisplay(bool) {}
  void dim(bool) {}
  void ssd1306_command(uint8_t) {}
  // Host-only.
  int host_frames() const { return frames_; }
  uint8_t host_address() const { return addr_; }
  bool host_begun() const { return begun_; }
 private:
  int frames_ = 0;
  uint8_t vcc_ = 0, addr_ = 0;
  bool begun_ = false;
};
