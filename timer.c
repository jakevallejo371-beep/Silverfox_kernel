#include "timer.h"
#include "asm.h"
#include "shell.h"

int init_timer(uint32_t frequency) {
    
    uint16_t divisor = PIT_FREQ / frequency;
    
    // Send command byte: Channel 0, lowbyte / highbyte, rate generator
    outb(PIT_COMMAND, 0x36);
    
    // Set the divisor (low byte, then high byte)
    outb(PIT_CHANNEL_0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL_0, (uint8_t)((divisor >> 8) & 0xFF));

    return 1;
}

volatile uint32_t ticks = 0;

/* Note: pm = true means PM, pm = false means AM */
struct clock system_clock = {0, 0, 0, 0, false};

void timer_handler(void) {
    ticks++;
    outb(PIC1_COMMAND, 0x20);

    // Calender logic
    system_clock.mic++;

    if (system_clock.mic >= 60)
    {
        system_clock.mic = 0;
        system_clock.mil++;
    }

    if (system_clock.mil >= 60)
    {
        system_clock.mil = 0;
        system_clock.sec++;
    }

    if (system_clock.sec >= 60)
    {
        system_clock.sec = 0;
        system_clock.min++;
    }

    if (system_clock.min >= 60)
    {
        system_clock.min = 0;
        system_clock.hour++;
    }

    if (system_clock.hour >= 24)
    {
        system_clock.hour =1;
    }
}