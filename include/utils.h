#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

int str_len(char* str);
char is_dec(char* str);
char is_hex(char* str);
char is_reg(char* str);

__int128 s2n(char* str);
__int128 h2n(char* str);

#endif // UTILS_H
