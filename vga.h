#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include "config.h"

// Struct type declaration must reside at top level
struct color {
    int back;
    int fore;
    int error;
    int warning;
    int keyword;
};

// Expose variables globally to match commands.c references
extern struct color theme;
extern unsigned int cursor_x;
extern unsigned int cursor_y;
extern volatile unsigned short* vga_buffer;

// Functional Prototypes
int calcVGA_color(struct color theme);
int enableHardwareCursor(uint8_t cursor_start, uint8_t cursor_end);
int disableBlink(void);
void updateHardwareCursor(int x, int y);
void clearScreen(int argc, const char** args);
void scroll(void);

// Core Print Routines
void print(const char* string);
void printCord(const char* string, int cur_x, int cur_y);
void printError(const char* string);
void printWarning(const char* string);
void printImportant(const char* string);
void printChar(const char c);
void printCharCommand(const char c);
void printCharCord(const char c, int cur_x, int cur_y);

#endif // VGA_H