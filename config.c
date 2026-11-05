#ifndef CONFIG_H
#define CONFIG_H

// Core limits
#define X32_BIT_LIMIT           4294967296ULL
#define RING_BUFFER_SIZE        6
#define HISTORY_SIZE            64
#define COMMAND_SIZE            100

// VGA Display config
#define VGA_WIDTH               80
#define VGA_HEIGHT              25
#define CURSOR_START_REGISTER   0x0A
#define VGA_CTRL_PORT           0x3D4
#define VGA_DATA_PORT           0x3D5
#define VGA_CURSOR_HIGH         0x0E
#define VGA_CURSOR_LOW          0x0F

// Keyboard interface
#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64

// Interrupt table gates
#define INT_GATE                0x8E
#define USER_GATE               0xEE

// Programmable Interrupt Controllers (PIC)
#define PIC1_COMMAND            0x20
#define PIC1_DATA               0x21
#define PIC2_COMMAND            0xA0
#define PIC2_DATA               0xA1

#define ICW1_INIT               0x10
#define ICW1_ICW4               0x01
#define ICW4_8086               0x01

// 16-Color VGA Palette Colors
#define HEX_BLACK               0x0
#define HEX_BLUE                0x1
#define HEX_GREEN               0x2
#define HEX_CYAN                0x3
#define HEX_RED                 0x4
#define HEX_MANGENTA            0x5
#define HEX_BROWN               0x6
#define HEX_LIGHT_GREY          0x7
#define HEX_DARK_GREY           0x8
#define HEX_LIGHT_BLUE          0x9
#define HEX_LIGHT_GREEN         0xA
#define HEX_LIGHT_CYAN          0xB
#define HEX_LIGHT_RED           0xC
#define HEX_LIGHT_MANGENTA      0xD
#define HEX_YELLOW              0xE
#define HEX_WHITE               0xF

// Programmable Interval Timer (PIT)
#define PIT_CHANNEL_0           0x40
#define PIT_COMMAND             0x43
#define PIT_FREQ                1193180

// Boot up
#define TOTAL_OP                11

#endif // CONFIG_H