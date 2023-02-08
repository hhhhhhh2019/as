#ifndef LEX_H
#define LEX_H

#include <token.h>

typedef struct {
	unsigned int count;
	Token* tokens;
} Lex_result;

Lex_result lex_parse(char *text, unsigned int size);


#endif // LEX_H
