#ifndef TOKEN_H
#define TOKEN_H

#define TYPE_UNDEFINED 0
#define TYPE_NEW_LINE 1
#define TYPE_PREPROCESSOR 2
#define TYPE_NUMBER 3
#define TYPE_HEX_NUMBER 4
#define TYPE_INSTRUCTION 5
#define TYPE_REGISTER 6
#define TYPE_LABEL 7

#define TYPE_SUM 100
#define TYPE_SUB 101
#define TYPE_MUL 102
#define TYPE_DIV 103
#define TYPE_BRACKET_START 104
#define TYPE_BRACKET_END 105
#define TYPE_ADDR_START 106
#define TYPE_ADDR_END 107
#define TYPE_GLOBAL_ADDR_START 108
#define TYPE_GLOBAL_ADDR_END 109

typedef struct {
	char type;
	unsigned int line;
	unsigned int offset;
	char *value;
} Token;

#endif // TOKEN_H
