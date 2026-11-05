#ifndef KEYBOARD_DRIVER_H
#define KEYBOARD_DRIVER_H

#include <stdbool.h>
#include "config.h"

// Global variable declarations (shared across files via extern)
extern volatile char ring_buffer[RING_BUFFER_SIZE];
extern volatile int head_elemt;
extern volatile int tail_elemt;

// Function blueprints
void keyboard_handler(void);
char scancodeToASCII(unsigned char scancode);
char* readline(void);
char* readCommand(void);

#endif
