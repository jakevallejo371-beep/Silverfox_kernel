#ifndef KSTR_H
#define KSTR_H

#include <stdbool.h>
#include "config.h"

// Translation & Formatting blueprints
char* int_to_str(int num);
int str_to_int(const char *str);

// Memory & Validation blueprints
bool strComp(const char* string1, const char* string2);
void strCpy(char* destination, const char* source);
int str_size(char* str);

#endif // KSTR_H
