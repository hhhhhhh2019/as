#include <lex.h>
#include <regex>


char asciitolower(char in) {
	if (in <= 'Z' && in >= 'A')
		return in - ('Z' - 'z');
	return in;
}


std::vector<std::string> instructions = {
	"hlt",
	"mov",
	"sum","sub","cmp",
	"push","pop","pushb","pushs","pushi","pushl","pushr",
	"jmp","je","jl","jb","jne","jle","jbe","call","ret","int","iret"
};


template<class T> char val_in_vector(std::vector<T> vec, T val) {
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i] == val) return 1;
	}

	return 0;
}


void Lex::clear() {
	tokens.clear();

	ok = 1;
}


void Lex::parse() {
	std::string val;
	char comment = 0;
	unsigned int line = 1;
	unsigned int offset = 1;

	for (int i = 0; i < code_length; i++) {
		char c = asciitolower(code[i]);

		if ((!('a' <= c && c <= 'z') && !('0' <= c && c <= '9') && c != '%' && c != '$' && c != ':' && c != '[' && c != ']' && c != '{' && c != '}' && c != '-') && comment == 0) {
			if (val.size() > 0) {
				Token token;

				token.value = val;
				token.line = line;
				token.offset = offset - val.size();

				char type = TYPE_UNDEFINED;

				if (val[0] == '%') {
					type = TYPE_PREPROCESSOR;
				} else if (regex_match(val, std::regex("-?[0-9]+"))) {
					type = TYPE_NUMBER;
				} else if (regex_match(val, std::regex("-?0x[0-9a-f]+"))) {
					type = TYPE_HEX_NUMBER;
				} else if (val_in_vector(instructions, val)) {
					type = TYPE_INSTRUCTION;
				} else if (regex_match(val, std::regex("[rlisb][0-9]+"))) {
					type = TYPE_REGISTER;
				} else if (regex_match(val, std::regex("\\{[0-9]+\\}"))) {
					type = TYPE_ADDR_WRITE;
				} else if (regex_match(val, std::regex("\\{0x[0-9a-f]+\\}"))) {
					type = TYPE_HEX_ADDR_WRITE;
				} else if (regex_match(val, std::regex("\\{[0-9]+\\}g"))) {
					type = TYPE_ADDR_WRITE_ABS;
				} else if (regex_match(val, std::regex("\\{0x[0-9a-f]+\\}a"))) {
					type = TYPE_HEX_ADDR_WRITE_ABS;
				} else if (regex_match(val, std::regex("\\[[0-9]+\\]"))) {
					type = TYPE_ADDR_READ;
				} else if (regex_match(val, std::regex("\\[0x[0-9a-f]+\\]"))) {
					type = TYPE_HEX_ADDR_READ;
				} else if (regex_match(val, std::regex("\\[[0-9]+\\]g"))) {
					type = TYPE_ADDR_READ_ABS;
				} else if (regex_match(val, std::regex("\\[0x[0-9a-f]+\\]a"))) {
					type = TYPE_HEX_ADDR_READ_ABS;
				} else if (val.back() == ':') {
					type = TYPE_LABEL;
				} else if (regex_match(val, std::regex("\\[.+\\]"))) {
					type = TYPE_LABEL_ADDR_READ;
				} else if (regex_match(val, std::regex("\\{.+\\}"))) {
					type = TYPE_LABEL_ADDR_WRITE;
				}

				token.type = type;

				tokens.push_back(token);
			}

			val.clear();
		} else if (comment == 0) {
			val += c;
		}

		if (c == '#')
			comment = 1;

		if (c == '\n') {
			comment = 0;
			line++;
			offset = 0;
		}

		offset++;
	}
}
