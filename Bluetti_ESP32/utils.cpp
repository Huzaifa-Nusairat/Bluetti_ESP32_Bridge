#include "utils.h"
#include "crc16.h"

uint16_t swap_bytes(uint16_t number) {
    return (number << 8) | (number >> 8);
}

uint16_t modbus_crc(uint8_t buf[], int len){
      unsigned int crc = 0xFFFF;
      for (unsigned int i = 0; i < len; i++)
       {
        crc = crc16_update(crc, buf[i]);
       }
     
     return crc;
}

bool modbus_reply_crc_ok(const uint8_t* buf, size_t len){
    if (buf == NULL || len < 5) {
        return false;
    }
    uint16_t expected = modbus_crc((uint8_t*)buf, (int)(len - 2));
    uint16_t received = (uint16_t)buf[len - 2] | ((uint16_t)buf[len - 1] << 8);  // low byte first
    return expected == received;
}

wrapper slice(const uint8_t* arr, int size, uint8_t include, uint8_t exclude) {
    wrapper result = { .myarr = {0}, .mysize = 0 };
    if (include >= 0 && exclude <= size) {
        int count = 0;
        for (int i = include; i < exclude; i++) {
            result.myarr[count] = arr[i];
            count++;
        }
        result.mysize = exclude - include;
        return result;
    }
    else {
        printf("Array index out-of-bounds\n");
        result.mysize = -1;
        return result;
    }
} 
