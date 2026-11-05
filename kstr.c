#include "kstr.h"

char* int_to_str(int num) {
    static char buffer[32];
    int i = 0;
    bool neg = false;

    if (num == 0) {
        buffer[i] = '0';
        i++;
        buffer[i] = '\0';
        return buffer;
    }

    if (num < 0) {
        neg = true;
        num = -num;
    }

    while (num > 0) {
        buffer[i++] = (char)('0' + (num % 10));
        num /= 10;
    }

    buffer[i] = '\0';

    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = temp;
    }
    
    if (neg) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';
    return buffer;
}

int str_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') {
        i++;
    }

    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    while (str[i] >= '0' && str[i] <= '9') {
        result = (result * 10) + (str[i] - '0');
        i++;
    }

    return result * sign;
}

bool strComp(const char* string1, const char* string2) {
    int i = 0;
    while (string1[i] && string2[i]) {
        if (string1[i] != string2[i]) {
            return false;
        }
        i++;
    }
    return string1[i] == '\0' && string2[i] == '\0';
}

void strCpy(char* destination, const char* source) {
    int i = 0;
    while (source[i] != '\0' && i < COMMAND_SIZE - 1) {
        destination[i] = source[i];
        i++;
    }
    destination[i] = '\0';
}

int str_size(char* str) {
    int c =0;
    while (str[c] != '\0')
        c++;
    return c;
}