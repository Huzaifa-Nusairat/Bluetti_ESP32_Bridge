#include "Adafruit_GFX.h"
#include "Wire.h"
#include "SPI.h"
#include <stdlib.h>
TwoWire Wire;
SPIClass SPI;
void Adafruit_GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
  for (;;) {
    drawPixel(x0, y0, color);
    if (x0 == x1 && y0 == y1) break;
    int e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; }
    if (e2 <= dx) { err += dx; y0 += sy; }
  }
}
void Adafruit_GFX::drawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int16_t w, int16_t h, uint16_t color) {
  int16_t byte_width = (w + 7) / 8;
  for (int16_t j = 0; j < h; ++j)
    for (int16_t i = 0; i < w; ++i)
      if (bitmap[j * byte_width + i / 8] & (0x80 >> (i & 7))) drawPixel(x + i, y + j, color);
}
