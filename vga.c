#include "vga.h"
#include "asm.h"
#include "shell.h"

//Important!
volatile unsigned short* vga_buffer = (unsigned short*)0xB8000;

unsigned int cursor_x = 0;
unsigned int cursor_y = 0;

struct color theme = { HEX_BLACK, HEX_WHITE, HEX_RED, HEX_YELLOW, HEX_MANGENTA };

int calcVGA_color(struct color theme) {
    return (theme.back << 4) | (theme.fore & 0x0F);
}

// 
int enableHardwareCursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(VGA_CTRL_PORT, CURSOR_START_REGISTER);
    // Fetch current register value, clear top bits, set scanlines, and ensure Bit 5 is 0 (0 = Enable, 1 = Disable)
    outb(VGA_DATA_PORT, (inb(VGA_DATA_PORT) & 0xC0) | cursor_start);
    
    // 0x0B is the Cursor End Register
    outb(VGA_CTRL_PORT, 0x0B);
    outb(VGA_DATA_PORT, (inb(VGA_DATA_PORT) & 0xE0) | cursor_end);

    return 1;
}

int disableBlink(void) {
    inb(0x3DA);                 // reset the attribute controller flip-flop
    outb(0x3C0, 0x30);          // select register 0x10 (Mode Control), keep display enabled
    unsigned char mode = (unsigned char)inb(0x3C1);
    mode &= ~0x08;              // bit 3 = blink enable
    outb(0x3C0, mode);

    return 1;
}

void updateHardwareCursor(int x, int y) {
    unsigned short pos = y * VGA_WIDTH + x;
    
    outb(VGA_CTRL_PORT, VGA_CURSOR_LOW);
    outb(VGA_DATA_PORT, (unsigned short)(pos & 0xFF));
    
    outb(VGA_CTRL_PORT, VGA_CURSOR_HIGH);
    outb(VGA_DATA_PORT, (unsigned short)((pos >> 8) & 0xFF));
}

void clearScreen(int argc, const char** args) {
    if (voidarg(argc))
        return;

    for (int i=0; i < 2000; i++)
    {
        vga_buffer[i] = (unsigned short)' ' | ((unsigned short)calcVGA_color(theme) << 8);
    }

    cursor_x = 0;
    cursor_y = 0;
}


void scroll(void) {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
    }

    int last_row_start = (VGA_HEIGHT - 1) * VGA_WIDTH;
    for (int i = 0; i < VGA_WIDTH; i++) {
        vga_buffer[last_row_start + i] = (unsigned short)' ' | ((unsigned short)calcVGA_color(theme) << 8);
    }

    cursor_y = VGA_HEIGHT - 1;

    updateHardwareCursor(cursor_x, cursor_y);
}

void printCord(const char* string, int cur_x, int cur_y) {
    int x = cur_x;
    int y = cur_y;

    for (int i=0; string[i] != '\0'; i++)
    {
        if (y >= VGA_HEIGHT)
        {
            scroll();
            y = VGA_HEIGHT - 1;
        }

        cursor_x = x;
        cursor_y = y;

        if (string[i] == '\n')
        {
            x = cur_x;
            y++;
            continue;
        }

        int index = (y * VGA_WIDTH) + (x++);
        vga_buffer[index] = (unsigned short)string[i] | ((unsigned short)calcVGA_color(theme) << 8);

        if (x >= VGA_WIDTH)
        {
            x = 0;
            y++;
        }

        updateHardwareCursor(x, y);
    }
}

void print(const char* string) {
    int x = cursor_x;
    int y = cursor_y;

    for (int i=0; string[i] != '\0'; i++)
    {
        if (x >= VGA_WIDTH)
        {
            x = 0;
            y++;
        }

        if (y >= VGA_HEIGHT)
        {
            scroll();
            y = VGA_HEIGHT - 1;
        }

        if (string[i] == '\n') {
            x = 0;
            y++;
            continue;
        }

        int index = (y * VGA_WIDTH) + x;
        vga_buffer[index] = (unsigned short)string[i] | ((unsigned short)calcVGA_color(theme) << 8);
        x++;
    }

    cursor_x = x;
    cursor_y = y;

    updateHardwareCursor(x, y);
}

void printError(const char* string) {
    int x = cursor_x;
    int y = cursor_y;

    for (int i=0; string[i] != '\0'; i++)
    {
        if (x >= VGA_WIDTH)
        {
            x = 0;
            y++;
        }

        if (y >= VGA_HEIGHT)
        {
            scroll();
            y = VGA_HEIGHT - 1;
        }

        if (string[i] == '\n') {
            x = 0;
            y++;
            continue;
        }

        int index = (y * VGA_WIDTH) + x;
        vga_buffer[index] = (unsigned short)string[i] | ((unsigned short)((theme.back << 4) | (theme.error & 0x0F)) << 8);
        x++;
    }

    cursor_x = x;
    cursor_y = y;

    updateHardwareCursor(x, y);
}

void printWarning(const char* string) {
    int x = cursor_x;
    int y = cursor_y;

    for (int i=0; string[i] != '\0'; i++)
    {
        if (x >= VGA_WIDTH)
        {
            x = 0;
            y++;
        }

        if (y >= VGA_HEIGHT)
        {
            scroll();
            y = VGA_HEIGHT - 1;
        }

        if (string[i] == '\n') {
            x = 0;
            y++;
            continue;
        }

        int index = (y * VGA_WIDTH) + x;
        vga_buffer[index] = (unsigned short)string[i] | ((unsigned short)((theme.back << 4) | (theme.warning & 0x0F)) << 8);
        x++;
    }

    cursor_x = x;
    cursor_y = y;

    updateHardwareCursor(x, y);
}

void printChar(const char c) {
    int x = cursor_x;
    int y = cursor_y;
    if (x >= VGA_WIDTH)
    {
        x = 0;
        y++;
    }

    if (y >= VGA_HEIGHT)
    {
        scroll();
        y = VGA_HEIGHT - 1;
    }

    if (c == '\n') {
        x = 0;
        y++;
    }

    int index = (y * VGA_WIDTH) + x;
    vga_buffer[index] = (unsigned short)c | ((unsigned short)calcVGA_color(theme) << 8);
    x++;

    cursor_x = x;
    cursor_y = y;

    updateHardwareCursor(x, y);
}

void printCharCommand(const char c) {
    int x = cursor_x;
    int y = cursor_y;
    if (x >= VGA_WIDTH)
    {
        x = 0;
        y++;
    }

    if (y >= VGA_HEIGHT)
    {
        scroll();
        y = VGA_HEIGHT - 1;
    }

    if (c == '\n') {
        x = 0;
        y++;
    }

    int index = (y * VGA_WIDTH) + x;
    vga_buffer[index] = (unsigned short)c | ((unsigned short)((theme.back << 4) | (theme.keyword & 0x0F)) << 8);
    x++;

    cursor_x = x;
    cursor_y = y;

    updateHardwareCursor(x, y);
}

void printCharCord(const char c, int cur_x, int cur_y) {
    int x = cur_x;
    int y = cur_y;
    if (x >= VGA_WIDTH)
    {
        x = 0;
        y++;
    }

    if (y >= VGA_HEIGHT)
    {
        scroll();
        y = VGA_HEIGHT - 1;
    }

    if (c == '\n') {
        x = 0;
        y++;
    }   

    int index = (y * VGA_WIDTH) + x;
    vga_buffer[index] = (unsigned short)c | ((unsigned short)calcVGA_color(theme) << 8);

    cursor_x = x;
    cursor_y = y;

    updateHardwareCursor(x, y);
}