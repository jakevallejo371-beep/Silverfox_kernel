#include <stddef.h>

#include "shell.h"
#include "acpi.h"
#include "asm.h"
#include "keyboardDriver.h"
#include "kstr.h"
#include "timer.h"
#include "vga.h"
#include "allocator.h"
#include "memory.h"
#include <time.h>

typedef void (*cmd_func)(int argc, const char** args);

#define cmd_count (sizeof(cmd_table) / sizeof(cmd_table[0]))

typedef struct
{
    const char *name;
    cmd_func fptr;
    bool args_flag;
    const char *outline;
} cmd_entry;

bool commands = true;

int paraCheck(int argc, const char** args) {
    if (argc > 0 && (args == NULL || args[0] == NULL))
    {
        printError("error: invalid parameters\n");
        return 0;
    }
    return 1;
}

bool voidarg(int argc) {
    if (argc != 0)
    {
        printError("Command doesn't take arguments");
        return true;
    } else
        return false;
}

void shutdown(int argc, const char** args)
{
    if (voidarg(argc))
        return;

    printWarning("Are you sure you want to shutdown?\n");
    print("[y/n]: ");
    char *answer = readline();
    if (strComp(answer, "y") || strComp(answer, "Y"))
    {
        commands = false;
        printWarning("System shuting down...");
        __asm__ volatile("cli");
        acpiPowerOff();
        printError("Error: failed to shutdown");
        return;
    }
    else if (strComp(answer, "n") || strComp(answer, "N"))
    {
        printWarning("canceling...\n");
        return;
    }
    else
    {
        printError("Invial responce\n");
        printWarning("canceling...\n");
    }
}

void printTitle(int argc, const char** args)
{
    if (voidarg(argc))
        return;

    print(
        "*--------------------------*\n"
        "SilverFox kernel - Yip shell\n"
        "*--------------------------*\n"
    );
}

void test_interrupt_handler_caller(int argc, const char** args)
{
    if (argc == 0)
    {
        print("Available interrupts:\n");
        print("  32 = timer\n");
        print("  33 = keyboard\n");
        print("  48 = test\n");
        print("  50 = OOM\n");
        return;
    }

    if (argc != 1)
    {
        printError("usage: interrupt <vector>\n");
        return;
    }

    int vector = str_to_int(args[1]);
    printWarning("Calling interrupt vector: ");
    print(args[1]);
    print("...\n");

    switch (vector)
    {
    case 48:
        trigger_interrupt(0x30);
        break;
    case 33:
        trigger_interrupt(0x21);
        break;
    case 32:
        trigger_interrupt(0x20);
        break;
    case 50:
        trigger_interrupt(0x32);
        break;

    default:
        printError("Couldn't find interrupt ");
        printError(args[1]);
        printError(" in IDT\n");
        break;
    }
}

void test_interrupt_handler_output(int argc, const char** args)
{
    if (voidarg(argc))
        return;

    print("interupt test succesful\n");
}

void echo(int argc, const char** args)
{
    for (int i=1; i <= argc; i++)
    {
        print(args[i]);
        print(" ");
    }
    print("\n");
}

void reboot(int argc, const char** args) {
    if (voidarg(argc))
        return;

    printWarning("Are you sure you want to reboot?\n");
    print("[y/n]: ");
    char *answer = readline();
    if (strComp(answer, "y") || strComp(answer, "Y"))
    {
        printWarning("rebooting down...");
        outb(0x64, 0xFE); // Pulse CPU reset line
    }
    else if (strComp(answer, "n") || strComp(answer, "N"))
    {
        printWarning("canceling...\n");
        return;
    }
    else
    {
        printError("Invial responce\n");
        printWarning("canceling...\n");
    }
}

void VGA_color(int argc, const char** args) {
    if (0 == paraCheck(argc, args))
    {
        print("Themes: default, dark, light, retro_green, retro_orange, and ash\n");
        return;
    }

        else if (strComp((char *)args[1], "default"))
    {
        theme.back = HEX_BLACK;
        theme.fore = HEX_WHITE;
        theme.error = HEX_RED;
        theme.warning = HEX_YELLOW;
        theme.keyword = HEX_MANGENTA;
        clearScreen(0, NULL);
        printTitle(0, NULL);
    }

    else if (strComp((char *)args[1], "dark"))
    {
        theme.back = HEX_BLACK;
        theme.fore = HEX_WHITE;
        theme.error = HEX_RED;
        theme.warning = HEX_YELLOW;
        theme.keyword = HEX_GREEN;
        clearScreen(0, NULL);
        printTitle(0, NULL);
    }

    else if (strComp((char *)args[1], "light"))
    {
        theme.back = HEX_WHITE;
        theme.fore = HEX_BLACK;
        theme.error = HEX_RED;
        theme.warning = HEX_YELLOW;
        theme.keyword = HEX_GREEN;
        clearScreen(0, NULL);
        printTitle(0, NULL);
    }

    else if (strComp((char *)args[1], "retro_green"))
    {
        theme.back = HEX_BLACK;
        theme.fore = HEX_GREEN;
        theme.error = HEX_RED;
        theme.warning = HEX_LIGHT_BLUE;
        theme.keyword = HEX_YELLOW;
        clearScreen(0, NULL);
        printTitle(0, NULL);
    }

    else if (strComp((char *)args[1], "retro_orange"))
    {
        theme.back = HEX_BLACK;
        theme.fore = HEX_YELLOW;
        theme.error = HEX_RED;
        theme.warning = HEX_MANGENTA;
        theme.keyword = HEX_WHITE;
        clearScreen(0, NULL);
        printTitle(0, NULL);
    }

    else if (strComp((char *)args[1], "ash"))
    {
        theme.back = HEX_LIGHT_GREY;
        theme.fore = HEX_BLACK;
        theme.error = HEX_RED;
        theme.warning = HEX_YELLOW;
        theme.keyword = HEX_WHITE;
        clearScreen(0, NULL);
        printTitle(0, NULL);
    }

    else
    {
        printError(args[1]);
        printError(" is not a theme\n");
    }
}

int history_cursor = 0;
char cmd_history[HISTORY_SIZE][COMMAND_SIZE];

void print_history(int argc, const char** args) {
    if (0 == paraCheck(argc, args))
    {
        int j = 1;
        for (int i = 0; i < HISTORY_SIZE; i++)
        {
            if (cmd_history[i][0] != '\0')
            {
                print(int_to_str(j));
                j++;
                print("] ");
                print(cmd_history[i]);
                print("\n");
            }
        }
    }
    else if (strComp((char *)args[1], "cursor"))
    {
        print("DEBUG history_cursor = ");
        print(int_to_str(history_cursor));
        print("\n");
    }
}

void cmd_info(int argc, const char** args) {
    if (0 == paraCheck(argc, args))
    {
        print("Arguments: status, credit, and version\n");
    }

    if (strComp((char *)args[1], "status"))
    {
        print(
            "Architecture:       i386\n"
            "PIC:                remaped\n"
            "Interrupts:         enabled\n"
            "keyboard driver:    endbled\n"
            "VGA dimensions:     80x25\n");
    }
    else if (strComp((char *)args[1], "credit"))
    {
        print(
            "Dev name       | Worked on\n"
            "---------------|----------------\n"
            "Jacob Vallejo  | kernel, kernel entry point, vga functions, IDT,\n"
            "               | config, keyboard drivers, shell, terminal, \n"
            "               | string functions, event handlers: timer,\n"
        );
    }
    else if (strComp((char *)args[1], "version"))
    {
        print(
            "     Silverfox::Alpha\n"
            "+------------------------+\n"
            "Version:               1.X\n"
            "CPU:                32-bit\n"
            "Bootloader:   Multiboot v1\n"
            "Kernel type:          .iso\n"
            "Languages:    C and x86-32\n"
            "Total lines of code: 2,133\n"
        );
    }
}

void cmd_sleep(int argc, const char** args) {
    if (argc == 0)
    {
        return;
    }

    ticks = 0;
    uint32_t second_time = ((str_to_int(args[1])) * 1000) -2;

    while (ticks <= second_time) {}

    print("done\n");
}

struct date
{
    uint8_t month;
    uint8_t day;
    uint16_t year;
};

struct date today = {3, 25, 2010};

void cmd_date(int argc, const char** args) {
    if (argc > 2)
        return;

    if (strComp((char *)args[1], "set"))
    {
        print("Month: ");
        today.month = str_to_int((char *)readline());

        print("Day: ");
        today.day = str_to_int((char *)readline());

        print("Year: ");
        today.year = str_to_int((char *)readline());
        print("\n");
    }
    else
    {
        print(int_to_str(today.month));
        print("/");
        print(int_to_str(today.day));
        print("/");
        print(int_to_str(today.year));
        print("\n");
    }
}

void cmd_time(int argc, const char** args)
{
    if (argc > 2)
     return;

    if (strComp(args[1], "set"))
    {
        print("Hour: ");
        system_clock.hour = str_to_int((char *)readline());

        print("Minute: ");
        system_clock.min = str_to_int((char *)readline());

        print("Second: ");
        system_clock.sec = str_to_int((char *)readline());
        print("\n");
    }
    else if (strComp(args[1], "military"))
    {
        print(int_to_str(system_clock.hour));
        print(":");
        print(int_to_str(system_clock.min));
    }
    else if (strComp(args[1], "standerd"))
    {
        if (system_clock.hour > 12)
            print(int_to_str(system_clock.hour-12));
        else
            print(int_to_str(system_clock.hour));
        print(":");
        print(int_to_str(system_clock.min));
        if (system_clock.pm == true)
            print(" PM\n");
        else
            print(" AM\n");
    }
    else
    {
        print(int_to_str(system_clock.hour));
        print(":");
        print(int_to_str(system_clock.min));
        print(":");
        print(int_to_str(system_clock.sec));
        print(":");
        print(int_to_str(system_clock.mil));
        print(":");
        print(int_to_str(system_clock.mic));
        if (system_clock.pm == true)
            print(" PM\n");
        else
            print(" AM\n");
    }
}

void cmd_test_alloc(void) {
    int* x = bump(&arena, sizeof(int));
    int* y = bump(&arena, sizeof(int));

    *x =1;
    *y =3;

    print("x = ");
    print(int_to_str(*x));
    print("\n");
    print("y = ");
    print(int_to_str(*y));
    print("\n");
}

void cmd_mem(void) {
    size_t free_bytes = arena.capacity - arena.offset;

    print("Total: ");
    print(int_to_str((int)arena.capacity));
    print("\nUsed: ");
    print(int_to_str((int)arena.offset));
    print("\nFree: ");
    print(int_to_str((int)free_bytes));
    print("\n\n");
}

void cmd_reset_bump(void) {
    bump_reset(&arena);
    print("bump allocator memory has been reset\n");
}

void cmd_alloc(int argc, const char** args) {
    if (0 == paraCheck(argc, args))
        return;

    if (argc == 0)
    {
        print("arguments: test, check, and reset");
        return;
    }

    if (argc == 1)
    {
        if (strComp(args[1], "test"))
        {
            cmd_test_alloc();
        }
        else if (strComp(args[1], "check"))
        {
            cmd_mem();
        }
        else if (strComp(args[1], "reset"))
        {
            cmd_reset_bump();
        }
        else {
            printError("unknown argument\n");
        }
        return;
    }
}

static const cmd_entry cmd_table[] = {
    {"help",       help,                          false, "[command_name] Displays information"},
    {"poweroff",   shutdown,                      false, "Shuts down the OS with Alias"},
    {"restart",    reboot,                        false, "Reboots the system"},
    {"clear",      clearScreen,                   false, "Clears the screen"},
    {"logo",       printTitle,                    false, "Displays logo"},
    {"theme",      VGA_color,                     true,  "[THEME_NAME] Changes theme"},
    {"print",      echo,                          true,  "<text...> Prints the argument"},
    {"intr",       test_interrupt_handler_caller, false, "[INTERRUPT_NUM] Tells the CPU to do the interrpt in (integer)argument"},
    {"system",     cmd_info,                      true,  "[ACTION] Prints info information about the computer/os"},
    {"history",    print_history,                 true,  "Shows command history"},
    {"wait",       cmd_sleep,                     true,  "<SECONDS> Wait for (seconds)argument"},
    {"date",       cmd_date,                      true,  "[set] prints or sets date"},
    {"time",       cmd_time,                      true,  "[set] prints or sets time"},
    {"memory",     cmd_alloc,                     true,  "[ACTION] Allocator settings"}
};

void help(int argc, const char** args)
{
    if (0 == paraCheck(argc, args))
        return;

    if (argc == 0)
    {
        for (size_t i = 0; i < cmd_count; i++)
        {
            print(cmd_table[i].name);
            print(" -> ");
            print(cmd_table[i].outline);
            print("\n");
        }
        return;
    }

    for (size_t i = 0; i < cmd_count; i++)
    {
        if (strComp((char *)args[1], (char *)cmd_table[i].name))
        {
            print(cmd_table[i].outline);
            print("\n");
            return;
        }
    }
    printError("No command named ");
    printError(args[1]);
    printError(" found\n");
}

char* findCmd(char *input)
{
    if (input == NULL)
    {
        return NULL;
    }

    // Ignore commands with only spaces
    int space_c =0;
    for (int i=0; i < str_size(input); i++)
    {
        if (input[i] == ' ')
            space_c++;
    }
    if (space_c == str_size(input))
        return NULL;

    // Turn command string into tokens
    char* tokens[12];
    int tk_index =0;

    int i=0;
    bool is_word = false;
    while (input[i] != '\0')
    {
        if (input[i] == ' ')
        {
            input[i] = '\0';
            is_word = false;
        }
        else if (input[i] != ' ' && is_word == false)
        {
            tokens[tk_index] = &input[i];
            tk_index++;
            is_word = true;
        }
        i++;
    }

    if (tk_index == 0)
        return NULL;

    // Find token in cmd_table
    bool real_cmd = false;
    int count_cmd = cmd_count;
    for (int i=0; i < count_cmd; i++)
    {
        if (strComp(tokens[0], cmd_table[i].name))
        {
            real_cmd = true;
            cmd_table[i].fptr(tk_index -1, (const char**)tokens);
            return NULL;
        }
    }

    if (!real_cmd)
        return "Error: unrecognized command :(";

    return NULL;
}