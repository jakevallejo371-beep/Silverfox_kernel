#include "keyboardDriver.h"
#include "asm.h"
#include "vga.h"

volatile char ring_buffer[6];

volatile int head_elemt = 0;
volatile int tail_elemt = 0;


void keyboard_handler(void) {
    ring_buffer[head_elemt] = inb(KEYBOARD_DATA_PORT);
    head_elemt = (head_elemt + 1) % RING_BUFFER_SIZE;
    
    //send the end of interrput
    outb(PIC1_COMMAND, 0x20);
};

char scancodeToASCII(unsigned char scancode) {
    static bool shift = false;

    //lower case
    const char map[128] = {
        0,  0, '1','2','3','4','5','6','7','8','9','0','-','=',
        '\b','\t','q','w','e','r','t','y','u','i','o','p','[',']',
        '\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',
        0,'\\','z','x','c','v','b','n','m',',','.','/',0,0,0,
        ' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    //upper case
    const char shift_map[128] = {
        0,  0, '!','@','#','$','%','^','&','*','(',')','_','+',
        '\b','\t','Q','W','E','R','T','Y','U','I','O','P','{','}',
        '\n',0,'A','S','D','F','G','H','J','K','L',':','"','~',
        0,'|','Z','X','C','V','B','N','M','<','>','?',0,0,0,
        ' ',0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    //if shift was released
    if (scancode & 0x80)
    {
        unsigned char releaseScancode = scancode & 0x7F;
        if (releaseScancode == 0x2A || releaseScancode == 0x36)
        {
            shift = false;
        }
        return 0;
    }

    //if shift pressed
    if (scancode == 0x2A || scancode == 0x36) {
        shift = true;
        return 0; 
    }

    //cap lock released
    if (scancode & 0x80)
    {
        unsigned char releaseScancode = scancode & 0x7F;
        if (releaseScancode == 0x3A)
        {
            shift = false;  
        }
        return 0;
    }

    //cap lock pressed
    if (scancode == 0x3A)
    {
        shift = !shift;
        return 0;
    }

    return (shift) ? shift_map[scancode] : map[scancode];
}

char* readline(void) {
    static char keys[100];
    int i = 0;
    char c = '\0';
    while (1)
    {
        if (i == 99)
        {
            print("ERROR: key array limit reached");
            return keys;
        }

        while (head_elemt == tail_elemt)
            hlt();

        c = scancodeToASCII(ring_buffer[tail_elemt]);
        tail_elemt = (tail_elemt + 1) % RING_BUFFER_SIZE;

        if (c <= 0) { continue; }

        if (c == '\n')
        {
            keys[i] = '\0';
            print("\n");
            return keys;
        }

        if (c == '\b' && i > 0)
        {
            i--;
            keys[i] = '\0';
            if  (cursor_x == 0)
            {
                cursor_x = VGA_WIDTH -1;
                cursor_y--;
            } else {
                cursor_x--;
            }
            printCharCord(' ', cursor_x, cursor_y);
        }
        else if (c == '\b' && i == 0) { continue;} 
        else {
            keys[i] = c;
            i++;
            printChar(c);
        }
    }
}

char* readCommand(void) {
    static char keys[100];
    int i = 0;
    char c = '\0';
    unsigned int typingCord = -1;
    while (1)
    {
        if (i == 99)
        {
            print("ERROR: key array limit reached");
            return keys;
        }

        while (head_elemt == tail_elemt)
            hlt();

        c = scancodeToASCII(ring_buffer[tail_elemt]);
        tail_elemt = (tail_elemt + 1) % RING_BUFFER_SIZE;

        if (c <= 0) { continue; }

        if (c == '\n')
        {
            keys[i] = '\0';
            print("\n");
            return keys;
        }

        if (c == 0x20)
            typingCord = cursor_x;

        if (c == '\b' && i > 0)
        {
            i--;
            keys[i] = '\0';
            if  (cursor_x == 0)
            {
                cursor_x = VGA_WIDTH -1;
                cursor_y--;
                typingCord = cursor_x;;
            } else {
                cursor_x--;
            }
            printCharCord(' ', cursor_x, cursor_y);

        }
        else if (c == '\b' && i == 0) { continue;} 
        else {
            keys[i] = c;
            i++;
            (cursor_x <= typingCord) ? printCharCommand(c) : printChar(c);
        }
    }
}