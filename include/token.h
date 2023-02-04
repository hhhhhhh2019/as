#pragma once

#include <string>



#define TYPE_UNDEFINED 0
#define TYPE_PREPROCESSOR 1
#define TYPE_NUMBER 2
#define TYPE_HEX_NUMBER 3
#define TYPE_INSTRUCTION 4
#define TYPE_REGISTER 5
#define TYPE_ADDR_WRITE 6
#define TYPE_ADDR_READ 7
#define TYPE_HEX_ADDR_WRITE 8
#define TYPE_HEX_ADDR_READ 9
#define TYPE_ADDR_WRITE_ABS 10
#define TYPE_ADDR_READ_ABS 11
#define TYPE_HEX_ADDR_WRITE_ABS 12
#define TYPE_HEX_ADDR_READ_ABS 13
#define TYPE_LABEL 14
#define TYPE_LABEL_ADDR_WRITE 15
#define TYPE_LABEL_ADDR_READ 16

struct Token {
	std::string value;
	char type = TYPE_UNDEFINED;
	unsigned int offset;
	unsigned int line;
};
