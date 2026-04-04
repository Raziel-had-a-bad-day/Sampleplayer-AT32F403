
#ifndef SAMPLER_LOADER_H
#define SAMPLER_LOADER_H

#include "at32f403a_407.h"


#define MAX_SAMPLES     32
#define MAX_NAME_LEN    16

typedef struct {
    char     name[MAX_NAME_LEN];
    uint32_t psram_addr;      // starting address in PSRAM
    uint32_t size_bytes;      // must be even (16-bit samples)
    uint8_t  used;
} SampleEntry;

extern SampleEntry sample_table[MAX_SAMPLES];

void sampler_loader_init(void);
void process_uart_command(void);
void uart_receive_char(char c);        // call this from your UART RX interrupt

// For playing a sample (you'll connect this later)
void play_sample(uint8_t id);

#endif
