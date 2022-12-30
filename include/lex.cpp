#pragma once


#ifndef DEBUG
	#define LOG
#else
	#define LOG(...) printf(__VA_ARGS__)
#endif


#include <token.cpp>
#include <utils.cpp>
#include <vector>
#include <string>
#include <regex>


std::vector<std::string> instructions = {
	"hlt",
	"mov",
	"push","pushr","pushl","pushi","pushs","pushb",
	"pop",
	"sum","sub","cmp",
	"jmp","je","jne","jl","jb","jle","jbe",
	"call","ret",
};


struct Lex {
	std::vector<Token> tokens;

	char parse(char *&data, unsigned int len) {
		tokens.clear();

		char ok = 1;

		std::string val;
		unsigned int line = 1;
		unsigned int offset = 1;
		unsigned int i = 0;

		bool is_comment = 0;

		while (i < len) {
			char c = data[i++];

			if ((c == '\n' || c == '\t' || c == ' ' || c == '#') && is_comment == 0 && val.size() != 0) {
				Token t;
				t.line = line - 1;
				t.offset = offset - val.size();
				t.value = val;

				char type = TOKEN_TYPE::undefined;


				if (std::regex_match(val, std::regex("[0-9]+")))
					type = TOKEN_TYPE::number;

				if (std::regex_match(val, std::regex("0x[a-f0-9]+")))
					type = TOKEN_TYPE::hex_number;

				if (is_register(val))
					type = TOKEN_TYPE::reg;

				if (is_value_in_vector(val,instructions))
					type = TOKEN_TYPE::instruction;

				if (std::regex_match(val, std::regex("\\[[0-9]+\\]")))
					type = TOKEN_TYPE::addr_in;

				if (std::regex_match(val, std::regex("\\[0x[0-9]+\\]")))
					type = TOKEN_TYPE::addr_in_hex;

				if (std::regex_match(val, std::regex("\\{[0-9]+\\}")))
					type = TOKEN_TYPE::addr_out_hex;

				if (std::regex_match(val, std::regex("\\{0x[0-9]+\\}")))
					type = TOKEN_TYPE::addr_out_hex;

				if (val[0] == '%')
					type = TOKEN_TYPE::preprocessor;

				if (val[val.size()-1] == ':')
					type = TOKEN_TYPE::label;


				t.type = type;

				tokens.push_back(t);


				if (type == TOKEN_TYPE::undefined) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
							line,offset-val.size(), "Неизвестный тип!", val.c_str());
					ok = 0;
				}

				val = "";

				if (c == '\n') {
					line++;
					offset = 1;
				}

				continue;
			} else {
				offset++;

				if (c != '\n' && c != ' ' && c != '\t')
					val += c;
			}

			if (c == '\n') {
				is_comment = 0;
				val = "";
			}

			if (c == '#')
				is_comment = 1;
		}

		return ok;
	}
};
