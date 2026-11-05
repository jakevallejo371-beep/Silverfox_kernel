#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

// Share the global tick tracking variable with other modules (like your sleep command)
extern volatile uint32_t ticks;

// Clock structure
struct clock {
    int hour;
    int min;
    int sec;
    int mil;
    int mic;
    bool pm;
};

extern struct clock system_clock;

// Function blueprints
int init_timer(uint32_t frequency);
void timer_handler(void);

#endif // TIMER_H
