#include <utils.h>
#include <token.h>


int str_len(char* str) {
	int res = 0;
	while (*str++) res++;
	return res;
}


char is_dec(char* str) {
	if (!*str) return 0;

	if (*str == '-')
		str++;

	str--;

	while (*++str)
		if ('0' > *str || *str > '9')
			return 0;
	return 1;
}

char is_hex(char* str) {
	if (*(str++) != '0')
		return 0;
	if (*str != 'x')
		return 0;

	while (*++str)
		if (!('0' <= *str && *str <= '9') && !('a' <= *str && *str <= 'f'))
			return 0;
	return 1;
}

char is_reg(char* str) {
	if (*str != 'r' && *str != 'l' && *str != 'i' && *str != 's' && *str != 'b')
		return 0;
	return is_dec(++str);
}


__int128 pow128(__int128 a, __int128 p) {
	__int128 r = 1;
	while (p-- > 0)
		r *= a;
	return r;
}

__int128 s2n(char* str) {
	__int128 res = 0;
	char sign = 0;
	if (*str == '-')
		sign = 1;
	int i = str_len(str) - sign - 1;
	while (*(str+sign))
		res += (*((str++)+sign) - '0') * pow128(10, i--);
	return res * -(sign * 2 - 1);
}

__int128 h2n(char* str) {
	__int128 res = 0;
	char sign = 0;
	if (*str == '-')
		sign = 1;
	int i = str_len(str) - sign - 3;
	str += 2 + sign;
	while (*(str+sign)) {
		if ('0' <= *str && *str <= '9')
			res += (*((str++)+sign) - '0') * pow128(16, i--);
		else
			res += (*((str++)+sign) - 'a' + 10) * pow128(16, i--);
	}
	return res * -(sign * 2 - 1);
}


void print_token_type(char t) {
	if (t == TYPE_UNDEFINED)
		printf("undefined\0");
	else if (t == TYPE_NUMBER || t == TYPE_HEX_NUMBER)
		printf("число\0");
	else if (t == TYPE_REGISTER)
		printf("регистр\0");
	else if (t == TYPE_INSTRUCTION)
		printf("инструкция\0");
	else if (t == TYPE_PREPROCESSOR)
		printf("препроце\0");
	else if (t == TYPE_LABEL)
		printf("метка\0");
	else if (t == TYPE_SUM || t == TYPE_SUB || t == TYPE_MUL || t == TYPE_DIV || t == TYPE_BRACKET_START || t == TYPE_BRACKET_END)
		printf("мат. операнд\0");
	else if (t == TYPE_ADDR_START || t == TYPE_GLOBAL_ADDR_START)
		printf("адрес\0");
}
