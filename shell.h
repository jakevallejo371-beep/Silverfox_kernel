#ifndef ROOTOS_SHELL_H
#define ROOTOS_SHELL_H

#include <stdbool.h>
#include "config.h"

extern bool commands;
extern int history_cursor;
extern char cmd_history[HISTORY_SIZE][COMMAND_SIZE];

int paraCheck(int argc, const char** args);
bool voidarg(int argc);
void shutdown(int argc, const char** args);
void printTitle(int argc, const char** args);
void test_interrupt_handler_caller(int argc, const char** args);
void test_interrupt_handler_output(int argc, const char** args);
void help(int argc, const char** args);
void echo(int argc, const char** args);
void reboot(int argc, const char** args);
void VGA_color(int argc, const char** args);
void version(int argc, const char** args);
void print_history(int argc, const char** args);
void status(int argc, const char** args);
void print_credit(int argc, const char** args);
void cmd_sleep(int argc, const char** args);
void cmd_date(int argc, const char** args);
char* findCmd(char* input);

#endif // COMMANDS_H
