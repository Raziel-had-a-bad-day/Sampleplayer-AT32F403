
#include "sampler_loader.h"

#include <string.h>
#include <stdio.h>

#include <stdbool.h>




// Simple command buffer
static char cmd_buffer[64];
static uint8_t cmd_idx = 0;

// Current upload state
static bool uploading = false;
static uint32_t upload_addr = 0;
static uint32_t upload_remaining = 0;
static char upload_name[MAX_NAME_LEN];

// =============================================
// Simple UART command handler (call from main loop or UART IRQ)
// =============================================


#include "sampler_loader.h"

#include <string.h>
#include <stdio.h>

SampleEntry sample_table[MAX_SAMPLES] = {0};

static char cmd_buffer[64];


// Upload state

static uint32_t upload_psram_addr = 0;

static char upload_name[MAX_NAME_LEN];
static uint32_t current_upload_slot = 0;

// =============================================
// Init
// =============================================
void sampler_loader_init(void)
{
    memset(sample_table, 0, sizeof(sample_table));
    printf("UART Sample Loader ready (115200 baud)\r\n");
    printf("Commands: list, upload name size, play id\r\n");
}

// =============================================
// Main command processor
// =============================================
void process_uart_command(void)
{
    if (cmd_idx == 0) return;
    cmd_buffer[cmd_idx] = '\0';

    if (uploading) {
        cmd_idx = 0;
        return;
    }

    // LIST
    if (strcmp(cmd_buffer, "list") == 0) {
        printf("ID  Name           Size\r\n");
        printf("------------------------\r\n");
        for (int i = 0; i < MAX_SAMPLES; i++) {
            if (sample_table[i].used) {
                printf("%2d  %-14s  %lu bytes\r\n",
                       i, sample_table[i].name, sample_table[i].size_bytes);
            }
        }
    }

    // UPLOAD name size
    else if (strncmp(cmd_buffer, "upload ", 7) == 0) {
        char name[16];
        uint32_t size;

        if (sscanf(cmd_buffer + 7, "%15s %lu", name, &size) == 2) {
            if (size == 0 || size > 524288) {           // max 512KB per sample
                printf("Error: Invalid size (max 512KB)\r\n");
                goto reset;
            }

            // Find free slot
            int slot = -1;
            for (int i = 0; i < MAX_SAMPLES; i++) {
                if (!sample_table[i].used) {
                    slot = i;
                    break;
                }
            }

            if (slot == -1) {
                printf("Error: No free slots\r\n");
                goto reset;
            }

            strncpy(upload_name, name, MAX_NAME_LEN-1);
            upload_name[MAX_NAME_LEN-1] = '\0';

            upload_psram_addr = 0x10000 + (slot * 0x80000);   // rough allocation in PSRAM
            upload_remaining = size;
            current_upload_slot = slot;
            uploading = true;

            printf("OK Ready to upload '%s' (%lu bytes). Send raw 16-bit data now...\r\n", name, size);
        }
    }

    // PLAY id
    else if (strncmp(cmd_buffer, "play ", 5) == 0) {
        int id;
        if (sscanf(cmd_buffer + 5, "%d", &id) == 1 && id >= 0 && id < MAX_SAMPLES) {
            if (sample_table[id].used) {
                printf("Playing sample %d: %s\r\n", id, sample_table[id].name);
                play_sample(id);
            } else {
                printf("Sample %d not found\r\n", id);
            }
        }
    }

reset:
    cmd_idx = 0;
}

// =============================================
// Call this from UART RX interrupt
// =============================================
void uart_receive_char(char c)
{
    if (uploading) {
        // Write byte to PSRAM
        // We'll implement proper 32-byte chunking in the next step
        // For now, this is placeholder
        return;
    }

    if (c == '\r' || c == '\n') {
        process_uart_command();
    } else if (cmd_idx < sizeof(cmd_buffer)-1) {
        cmd_buffer[cmd_idx++] = c;
    }
}


void play_sample(uint8_t id){
return;
};
