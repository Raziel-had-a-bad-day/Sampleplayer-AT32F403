// everything amplitude or gain related functions here , inc ADSR, filter curves etc
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float     current;
    float     low;
    float     high;
    float     start_value;
    uint32_t  start_time;
    uint32_t  duration;          // rate control (in your MTC / bar time units)
    bool      active;
} lfo_t;
lfo_t lfo;

void lfo_init(lfo_t *l, float low, float high, uint32_t duration)
{
    l->low         = low;
    l->high        = high;
    l->current     = low;
    l->start_value = low;
    l->duration    = duration;
    l->start_time  = 0;
    l->active      = false;
}

/* Change low and/or high smoothly – continues from current value */
void lfo_set_range(lfo_t *l, float new_low, float new_high, uint32_t now)
{
    if (new_high < new_low) {
        float tmp  = new_low;
        new_low    = new_high;
        new_high   = tmp;
    }

    l->low  = new_low;
    l->high = new_high;

    // Smooth: start a new ramp from wherever we are right now
    l->start_value = l->current;
    l->start_time  = now;
    l->active      = true;
}

/* Change the rate (duration) */
void lfo_set_rate(lfo_t *l, uint32_t new_duration, uint32_t now)
{
    if (new_duration < 1) {new_duration = 1;
    l->current = l->low;  // reset to start  on 0 set

    }
    // safety

    l->duration    = new_duration;
    // Recalculate so the remaining distance still finishes in the new duration
    l->start_value = l->current;
    l->start_time  = now;
    l->active      = true;
}

/* Convenience helpers */
void lfo_set_high(lfo_t *l, float new_high, uint32_t now)
{
    lfo_set_range(l, l->low, new_high, now);
}

void lfo_set_low(lfo_t *l, float new_low, uint32_t now)
{
    lfo_set_range(l, new_low, l->high, now);
}

float lfo_update(lfo_t *l, uint32_t now)
{
    if (!l->active)
        return l->current;

    // Protect against MTC time jumps / stop
    if (now < l->start_time) {
        l->start_value = l->current;
        l->start_time  = now;
    }

    uint32_t elapsed = now - l->start_time;

    if (elapsed >= l->duration) {
        // Finished → wrap to low and start next cycle immediately
        l->current     = l->low;
        l->start_value = l->low;
        l->start_time  = now;
        // stay active
        return l->current;
    }

    float progress = (float)elapsed / (float)l->duration;
    l->current = l->start_value + progress * (l->high - l->start_value);

    return l->current;
}

bool  lfo_is_active(const lfo_t *l) { return l->active; }
float lfo_get(const lfo_t *l)       { return l->current; }

