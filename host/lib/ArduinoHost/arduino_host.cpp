#include "Arduino.h"
#include <time.h>
// The clock: real elapsed time plus whatever delay() skipped (delay never sleeps, so tests stay fast).
static uint64_t offset_us = 0;
static uint64_t real_us() {
  static uint64_t start = 0;
  struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
  uint64_t now = (uint64_t)ts.tv_sec * 1000000u + (uint64_t)ts.tv_nsec / 1000u;
  if (start == 0) start = now;
  return now - start;
}
static uint64_t now_us();
EspClass ESP;
HardwareSerial Serial;
HardwareSerial Serial0;
HardwareSerial Serial1;
uint32_t millis() { return (uint32_t)(now_us() / 1000u); }
uint32_t micros() { return (uint32_t)now_us(); }
void delay(uint32_t ms) { offset_us += (uint64_t)ms * 1000u; now_us(); }
void host_set_millis(uint32_t ms) { uint64_t r = real_us(); offset_us = (uint64_t)ms * 1000u > r ? (uint64_t)ms * 1000u - r : 0; }
static int pins[256];
static int analog[256];
void pinMode(uint8_t, uint8_t) {}
void digitalWrite(uint8_t pin, uint8_t value) { pins[pin] = value; }
int digitalRead(uint8_t pin) { return pins[pin]; }
int analogRead(uint8_t pin) { return analog[pin]; }
void analogReadResolution(uint8_t) {}
void delayMicroseconds(uint32_t) {}
long random(long howbig) { return howbig > 0 ? ::random() % howbig : 0; }
long random(long howsmall, long howbig) { return howbig > howsmall ? howsmall + random(howbig - howsmall) : howsmall; }
void randomSeed(unsigned long seed) { srandom(seed); }
int host_pin_value(uint8_t pin) { return pins[pin]; }
void host_set_analog(uint8_t pin, int value) { analog[pin] = value; }
// Timers fire from the clock: whenever millis()/micros() is read, every alarm whose period has passed runs its
// callback (once per elapsed period), as the hardware would have between two reads.
struct hw_timer_s {
  void (*fn)(void) = nullptr; bool alarm = false, reload = false, used = false;
  uint32_t freq = 1000000; uint64_t period_us = 0, next_us = 0;
};
static hw_timer_s timers[4];
static bool in_timer = false;
static uint64_t now_us() {
  uint64_t now = real_us() + offset_us;
  if (in_timer) return now;
  in_timer = true;
  for (auto& t : timers) {
    while (t.used && t.fn && t.alarm && t.period_us && now >= t.next_us) {
      t.fn();
      if (!t.reload) { t.alarm = false; break; }
      t.next_us += t.period_us;
    }
  }
  in_timer = false;
  return now;
}
hw_timer_t* timerBegin(uint32_t frequency) {
  for (auto& t : timers) if (!t.used) { t = hw_timer_s(); t.used = true; t.freq = frequency ? frequency : 1; return &t; }
  return nullptr;
}
void timerEnd(hw_timer_t* t) { if (t) *t = hw_timer_s(); }
void timerAttachInterrupt(hw_timer_t* t, void (*fn)(void)) { if (t) t->fn = fn; }
void timerDetachInterrupt(hw_timer_t* t) { if (t) t->fn = nullptr; }
void timerAlarm(hw_timer_t* t, uint64_t alarm_value, bool autoreload, uint64_t) {
  if (!t) return;
  t->period_us = alarm_value * 1000000u / t->freq;
  t->reload = autoreload; t->alarm = true; t->next_us = real_us() + offset_us + t->period_us;
}
void host_fire_timers() { for (auto& t : timers) if (t.used && t.fn && t.alarm) t.fn(); }

// HOST_RANDOM_SEED makes random() repeatable per run (default 1, like an unseeded C library).
int main() {
  const char* seed = getenv("HOST_RANDOM_SEED");
  srandom(seed && *seed ? (unsigned)strtoul(seed, nullptr, 10) : 1u);
  setup();
  return 0;
}
