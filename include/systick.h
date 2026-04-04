// systick.h

#ifndef __SYSTICK_H
#define __SYSTICK_H

#include <stdint.h>

extern volatile uint32_t systick_ms;
extern volatile uint32_t systick_reload;

void systick_init(void);

#endif
