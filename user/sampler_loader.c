
#include "sampler_loader.h"

#include <string.h>
#include <stdio.h>
//#include "variables.h"
#include <stdbool.h>



#include "sampler_loader.h"
//#include "ram.h"          // your PSRAM write function
#include <string.h>
#include <stdio.h>
void handle_binary_upload(uint8_t byte);
// Sample table
SampleEntry sample_table[MAX_SAMPLES] = {0};
static char cmd_buffer[64];
  static uint8_t cmd_idx = 0;
// Private upload state
typedef struct {
    bool     active;
    uint32_t psram_addr;
    uint32_t remaining;
    char     name[MAX_NAME_LEN];
    int      slot;
    uint8_t  buffer[128];
    uint8_t  buf_idx;
    uint32_t original_size;   // for table entry
} UploadState;

static UploadState upload = {0};

// =============================================
// Init
// =============================================
void sampler_loader_init(void)
{
    memset(sample_table, 0, sizeof(sample_table));
    memset(&upload, 0, sizeof(upload));
   // printf("UART Sample Loader ready (115200 baud)\r\n");
 //   printf("Commands: list, upload <name> <size>, play <id>\r\n");
}



//SampleEntry sample_table[MAX_SAMPLES] = {0};
//
//static char cmd_buffer[64];
//static uint8_t cmd_idx = 0;
//
//// Upload state
//static bool uploading = false;
//static uint32_t upload_psram_addr = 0;
//static uint32_t upload_remaining = 0;
//static char upload_name[MAX_NAME_LEN];
//static uint32_t current_upload_slot = 0;

// =============================================
// Init
// =============================================
//-==========================================
// Main command processor
// =============================================
void process_uart_command(void)
{
    if (cmd_idx == 0) return;
    cmd_buffer[cmd_idx] = '\0';

//    if (uploading) {
//        cmd_idx = 0;
//        return;
//    }

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

				 strncpy(upload.name, name, MAX_NAME_LEN-1);
				 upload.psram_addr = 0x10000 + (slot * 0x40000);  // adjust allocation
				 upload.remaining = size;
				 upload.original_size = size;
				 upload.slot = 0;
				 upload.active = true;
				 upload.buf_idx = 0;
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


static void process_command(void)
{
     //... (same as before, but using upload struct)
//uint16_t size=32000;
//char name[]={"test"};
//uint8_t slot=0;
//    // Example for upload command:
//    if (strncmp(cmd_buffer, "upload ", 7) == 0) {
////        // ... parse name and size
//     //   int slot = find_free_slot();
//        if (slot >= 0) {
//             strncpy(upload.name, name, MAX_NAME_LEN-1);
//             upload.psram_addr = 0x10000 + (slot * 0x40000);  // adjust allocation
//             upload.remaining = size;
//             upload.original_size = size;
//             upload.slot = 0;
//             upload.active = true;
//             upload.buf_idx = 0;
//            // printf("OK Send raw 16-bit data for '%s' (%lu bytes)\r\n", name, size);
//         }
//     }
}

// =============================================
// UART character handler (call from ISR)
// =============================================
void uart_receive_char(char c)
{
   // static char cmd_buffer[64];
   // static uint8_t cmd_idx = 0;
	upload.active=true;
    if (upload.active) {
        handle_binary_upload((uint8_t)c);
        return;
    }

//    if (c == '\r' || c == '\n') {
//        cmd_buffer[cmd_idx] = '\0';
//        process_command();
//        cmd_idx = 0;
//    } else if (cmd_idx < sizeof(cmd_buffer)-1) {
//        cmd_buffer[cmd_idx++] = c;
//    }
}

// =============================================
// Binary upload handler
// =============================================
void handle_binary_upload(uint8_t byte)
{
    upload.buffer[upload.buf_idx++] = byte;

    if (upload.buf_idx == 128) {
        ram_page_write(upload.psram_addr, upload.buffer);
        upload.psram_addr += 128;
        upload.remaining -= 128;
        upload.buf_idx = 0;
    }

    if (upload.remaining == 0) {
        // Save to table
        int slot = upload.slot;
        strncpy(sample_table[slot].name, upload.name, MAX_NAME_LEN-1);
        sample_table[slot].name[MAX_NAME_LEN-1] = '\0';
        sample_table[slot].psram_addr = upload.psram_addr - upload.original_size;
        sample_table[slot].size_bytes = upload.original_size;
        sample_table[slot].used = 1;

    //    printf("Upload complete! Sample '%s' in slot %d\r\n", upload.name, slot);

        memset(&upload, 0, sizeof(upload));   // reset
    }
}

// Placeholder - connect to your playback later
void play_sample(uint8_t id)
{
    if (id < MAX_SAMPLES && sample_table[id].used) {
      //  printf("Playing %s from PSRAM 0x%08X (%lu bytes)\r\n",
           //    sample_table[id].name, sample_table[id].psram_addr, sample_table[id].size_bytes);
        // Call your actual playback start here
    }
}
