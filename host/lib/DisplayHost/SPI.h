// Host stand-in for the Arduino-ESP32 SPI library: only included through the display library.
#pragma once
#include "Arduino.h"
class SPIClass { public: void begin(int = -1, int = -1, int = -1, int = -1) {} };
extern SPIClass SPI;
