#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

int str_len(char*);
char is_dec(char*);
char is_hex(char*);
char is_reg(char*);

__int128 s2n(char*);
__int128 h2n(char*);

void print_token_type(char);

#endif // UTILS_H
