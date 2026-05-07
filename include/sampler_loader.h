#ifndef SAMPLER_LOADER_H
#define SAMPLER_LOADER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_SAMPLES     32
#define MAX_NAME_LEN    16

typedef struct {
    char     name[MAX_NAME_LEN];
    uint32_t psram_addr;      // start address in PSRAM
    uint32_t size_bytes;      // size in bytes (even for 16-bit)
    uint8_t  used;
} SampleEntry;

extern SampleEntry sample_table[MAX_SAMPLES];

void sampler_loader_init(void);
void uart_receive_char(char c);        // Call this from your UART RX interrupt
void play_sample(uint8_t id);          // You implement this later

#endif
