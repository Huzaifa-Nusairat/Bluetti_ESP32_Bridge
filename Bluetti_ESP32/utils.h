#ifndef UTILS_H
#define UTILS_H
#include "Arduino.h"

#define MAX 100

typedef struct{ 
    uint8_t myarr[MAX];
    int mysize;
} wrapper;

extern uint16_t swap_bytes(uint16_t number);
extern wrapper slice(const uint8_t* arr, int size, uint8_t include, uint8_t exclude);
extern uint16_t modbus_crc(uint8_t buf[], int len);
// true when buf is at least 5 bytes and its last two bytes (low byte first) are
// modbus_crc of every byte before them (owner decision 2026-10-19, hazard H8)
extern bool modbus_reply_crc_ok(const uint8_t* buf, size_t len);

#endif
