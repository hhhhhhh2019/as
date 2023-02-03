#pragma once

#include <vector>
#include <string>
#include <token.h>


struct Lex {
	std::vector<Token> tokens;
	char* code;
	unsigned int code_length;

	char ok;


	void clear();
	void parse();
};
