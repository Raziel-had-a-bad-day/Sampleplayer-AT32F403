// systick.c

#include "at32f403a_407.h"

volatile uint32_t systick_ms = 0;
volatile uint32_t systick_reload = 0;     // for microsecond calculation

// Put the SysTick_Handler here too
