#include <stddef.h>

#include "terminal.h"
#include "keyboardDriver.h"
#include "kstr.h"
#include "shell.h"
#include "vga.h"

void terminal(void) {
    char* inputBuffer = NULL;
    while (commands)
    {
        print("\n>");
        inputBuffer = readCommand();
        //add command to history
        strCpy(cmd_history[history_cursor], inputBuffer);
        char* error_str = findCmd(inputBuffer);
        if (error_str != NULL)
            printError(error_str);
        history_cursor = (history_cursor + 1) % HISTORY_SIZE;
    }
}