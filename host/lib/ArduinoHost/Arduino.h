// Host stand-in for the Arduino-ESP32 core: enough for pure-logic firmware modules and their Unity tests for host tests.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

typedef uint8_t byte;
typedef bool boolean;

uint32_t millis();
uint32_t micros();
void delay(uint32_t ms);
void host_set_millis(uint32_t ms);

#define PROGMEM
#define PGM_P const char*
#define F(s) (s)
#define FPSTR(p) (p)
#define pgm_read_byte(a) (*(const uint8_t*)(a))
#include "WString.h"
#include "Print.h"
extern HardwareSerial Serial;
extern HardwareSerial Serial0;
extern HardwareSerial Serial1;
#define USBSerial Serial

#define LOW 0
#define HIGH 1
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogReadResolution(uint8_t bits);
void delayMicroseconds(uint32_t us);
long random(long howbig);
long random(long howsmall, long howbig);
void randomSeed(unsigned long seed);
// Host-only: what the code wrote, and what analogRead returns.
int host_pin_value(uint8_t pin);
void host_set_analog(uint8_t pin, int value);

#include <time.h>
void configTime(long gmtOffset_sec, int daylightOffset_sec, const char* s1, const char* s2 = nullptr,
                const char* s3 = nullptr);
bool getLocalTime(struct tm* info, uint32_t ms = 5000);

class EspClass {
 public:
  void restart() {}
  uint32_t getFreeHeap() { return 200000; }
  uint32_t getMinFreeHeap() { return 150000; }
  uint32_t getMaxAllocHeap() { return 100000; }
  uint32_t getHeapSize() { return 320000; }
  uint64_t getEfuseMac() { return 0; }
  uint32_t getFlashChipSize() { return 4u * 1024u * 1024u; }
  const char* getSdkVersion() { return "host"; }
  uint32_t getCpuFreqMHz() { return 160; }
  const char* getChipModel() { return "host"; }
};
extern EspClass ESP;

// Hardware timers (ESP32 core 3.x API). Host: a timer never fires on its own; tests drive time explicitly.
#define IRAM_ATTR
typedef struct hw_timer_s hw_timer_t;
hw_timer_t* timerBegin(uint32_t frequency);
void timerEnd(hw_timer_t* timer);
void timerAttachInterrupt(hw_timer_t* timer, void (*fn)(void));
void timerDetachInterrupt(hw_timer_t* timer);
void timerAlarm(hw_timer_t* timer, uint64_t alarm_value, bool autoreload, uint64_t reload_count);
void host_fire_timers();  // host-only: run every attached timer callback once

#include "freertos_queue.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
void setup();
void loop();
