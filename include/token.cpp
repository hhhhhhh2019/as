#pragma once


enum TOKEN_TYPE {
	instruction,
	reg,
	number,
	hex_number,
	addr_in,
	addr_in_hex,
	addr_out,
	addr_out_hex,
	preprocessor,
	undefined
};

struct Token {
	std::string value;
	char type;
	unsigned int line;
	unsigned int offset;
};
