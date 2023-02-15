#include <lex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>


char instructions[][6] = {
	"nop\0\0\0", "hlt\0\0\0",
	"movb\0\0", "movs\0\0", "movi\0\0", "movl\0\0", "movr\0\0",
	"jmp\0\0\0", "je\0\0\0\0", "jl\0\0\0\0", "jb\0\0\0\0", "jne\0\0\0", "jbe\0\0\0", "jle\0\0\0",
	"call\0\0", "ret\0\0\0",
	"pushb\0", "pushs\0", "pushi\0", "pushl\0", "pushr\0",
	"popb\0\0", "pops\0\0", "popi\0\0", "popl\0\0", "popr\0\0"
};


char lower(char c) {
	if ('A' <= c && c <= 'Z')
		return c - 'A' + 'a';
	return c;
}

char is_stop_symb(char c) {
	return !('a' <= c && c <= 'z') && !('0' <= c && c <= '9') && c != '.' && c != ':' && c != '%';
}

char is_instr(char* str) {
	for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); i++) {
		if (strcmp(instructions[i], str) == 0)
			return 1;
	}

	return 0;
}


Lex_result lex_parse(char *text, unsigned int size) {
	Lex_result res;

	res.tokens = malloc(0);
	res.count = 0;

	char *value = malloc(0);
	unsigned int value_size = 0;

	unsigned int offset = 1;
	unsigned int line = 1;
	char comment = 0;

	for (int i = 0; i < size; i++) {
		char c = lower(text[i]);

		if (comment == 0) {
			if (is_stop_symb(c) == 1) {
				if (value_size > 0) {
					/*for (int j = 0; j < value_size; j++)
						putc(value[j], stdout);

					putc('\n', stdout);*/

					Token token;

					token.type = TYPE_UNDEFINED;
					token.line = line;
					token.offset = offset - value_size;
					token.value = malloc(value_size + 1);
					memcpy(token.value, value, value_size);
					token.value[value_size] = 0;

					if (is_dec(token.value)) {
						token.type = TYPE_NUMBER;
					} else if (is_hex(token.value)) {
						token.type = TYPE_HEX_NUMBER;
					} else if (token.value[0] == '%' && is_dec(&token.value[1])) {
						token.type = TYPE_REGISTER;
					} else if (is_instr(token.value)) {
						token.type = TYPE_INSTRUCTION;
					} else if (token.value[0] == '.') {
						token.type = TYPE_PREPROCESSOR;
					} else if (token.value[value_size - 1] == ':') {
						token.type = TYPE_LABEL;
					}

					res.tokens = realloc(res.tokens, (++res.count) * sizeof(Token));
					memcpy(&res.tokens[res.count - 1], &token, sizeof(Token));

					free(value);
					value = malloc(0);
					value_size = 0;
				}
			} else {
				value = realloc(value, ++value_size);
				value[value_size - 1] = c;
			}

			if (c == '+' || c == '-' || c == '*' || c == '/' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\n' || c == '(' || c == ')') {
				Token token;

				if (c == '+')
					token.type = TYPE_SUM;
				if (c == '-')
					token.type = TYPE_SUB;
				if (c == '*')
					token.type = TYPE_MUL;
				if (c == '/')
					token.type = TYPE_DIV;
				if (c == '[')
					token.type = TYPE_ADDR_START;
				if (c == ']')
					token.type = TYPE_ADDR_END;
				if (c == '{')
					token.type = TYPE_GLOBAL_ADDR_START;
				if (c == '}')
					token.type = TYPE_GLOBAL_ADDR_END;
				if (c == '\n')
					token.type = TYPE_NEW_LINE;
				if (c == '(')
					token.type = TYPE_BRACKET_START;
				if (c == ')')
					token.type = TYPE_BRACKET_END;
				token.line = line;
				token.offset = offset;
				token.value = malloc(2);
				token.value[0] = c;
				token.value[1] = 0;

				res.tokens = realloc(res.tokens, (++res.count) * sizeof(Token));
				memcpy(&res.tokens[res.count - 1], &token, sizeof(Token));
			}
		}

		offset++;

		if (c == ';') {
			comment = 1;
		}

		if (c == '\n') {
			Token token;
			token.line = line;
			token.offset = offset;
			token.value = malloc(2);
			token.value[0] = '\n';
			token.value[1] = 0;
			token.type = TYPE_NEW_LINE;

			res.tokens = realloc(res.tokens, (++res.count) * sizeof(Token));
			memcpy(&res.tokens[res.count - 1], &token, sizeof(Token));


			comment = 0;
			line++;
			offset = 1;
		}
	}

	free(value);

	if (res.tokens[res.count - 1].type != TYPE_NEW_LINE) {
		Token token = {TYPE_NEW_LINE, line,offset};
		token.value = malloc(2);
		token.value[0] = '\n';
		token.value[1] = 0;

		res.tokens = realloc(res.tokens, (++res.count) * sizeof(Token));
		memcpy(&res.tokens[res.count - 1], &token, sizeof(Token));
	}

	return res;
}
