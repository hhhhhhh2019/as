#pragma once


#include <token.h>
#include <vector>



struct Label {
	std::string value;
	unsigned long offset;
};


struct Synt {
	std::vector<Label> labels;
	std::vector<Label> alabels;
	std::vector<Token> tokens;
	std::vector<char> output;

	unsigned long org;

	void clear();
	char parse();
	char parse_labels();
};
