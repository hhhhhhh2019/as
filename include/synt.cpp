#pragma once


#ifndef DEBUG
	#define LOG
#else
	#define LOG(...) printf(__VA_ARGS__)
#endif


#include <token.cpp>
#include <utils.cpp>
#include <types.cpp>
#include <vector>


struct Synt {
	std::vector<char> output;

	char parse(std::vector<Token> tokens) {
		char ok = 1;
		unsigned int i = 0;

		unsigned long offset = 0;

		output.clear();

		while (i < tokens.size()) {
			Token t1 = tokens[i++];

			if (t1.type == preprocessor) {
				if (t1.value == "%org") {
					Token t2 = tokens[i++];

					if (is_hex(t2.value))
						offset = hex_string_to_number128(t2.value);
					else
						offset = string_to_number128(t2.value);

					printf("%lu %s\n", offset, t2.value.c_str());
				}

				continue;
			}

			if (t1.type != instruction) {
				printf("%i,%i \e[1;31m%-6s\e[m %s\n",
					t1.line,t1.offset, "Ожидается инструкция!", t1.value.c_str());
				ok = 0;
				continue;
			}


			if (t1.value == "hlt") {
				output.push_back(0xff);
				output.push_back(0xff);
				continue;
			}

			if (t1.value == "mov") {
				output.push_back(0x00);

				Token t2 = tokens[i++];

				if (t2.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается регистр!", t2.value.c_str());
					ok = 0;
					continue;
				}

				Token t3 = tokens[i++];

				if (t3.type == reg) {
					output.push_back(0x00);
					output.push_back(register_id(t2.value));
					output.push_back(register_id(t3.value));
				} else if (t3.type == number) {
					output.push_back(0x01);
					output.push_back(register_id(t2.value));
					uint128 n = string_to_number128(t3.value);
					for (int i = 0; i < register_size(t2.value); i++)
						output.push_back((n >> i * 8) & 0xff);
				} else if (t3.type == hex_number) {
					output.push_back(0x01);
					output.push_back(register_id(t2.value));
					uint128 n = hex_string_to_number128(t3.value);
					for (int i = 0; i < register_size(t2.value); i++)
						output.push_back((n >> i * 8) & 0xff);
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t3.line,t3.offset, "Ожидается регистр/число/адрес!", t3.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "push") {
				Token t2 = tokens[i++];

				if (t2.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается регистр!", t2.value.c_str());
					ok = 0;
					continue;
				}

				output.push_back(0x02);
				output.push_back(0x00);
				output.push_back(register_id(t2.value));
			}

			if (t1.value == "pushr") {
				Token t2 = tokens[i++];

				output.push_back(0x02);
				output.push_back(0x02);

				output.push_back(0x00);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value);
					for (int i = 0; i < 16; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value);
					for (int i = 0; i < 16; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "pushl") {
				Token t2 = tokens[i++];

				output.push_back(0x02);
				output.push_back(0x03);

				output.push_back(0x00);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value);
					for (int i = 0; i < 8; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value);
					for (int i = 0; i < 8; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "pushi") {
				Token t2 = tokens[i++];

				output.push_back(0x02);
				output.push_back(0x04);

				output.push_back(0x00);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value);
					for (int i = 0; i < 4; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value);
					for (int i = 0; i < 4; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "pushs") {
				Token t2 = tokens[i++];

				output.push_back(0x02);
				output.push_back(0x05);

				output.push_back(0x00);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value);
					for (int i = 0; i < 2; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value);
					for (int i = 0; i < 2; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "pushb") {
				Token t2 = tokens[i++];

				output.push_back(0x02);
				output.push_back(0x06);

				output.push_back(0x00);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value);
					for (int i = 0; i < 1; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value);
					for (int i = 0; i < 1; i++)
						output.push_back((n >> i * 8) & 0xff);
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "pop") {
				Token t2 = tokens[i++];

				output.push_back(0x02);
				output.push_back(0x01);

				output.push_back(register_id(t2.value));
			}

			if (t1.value == "sum") {
				Token t2 = tokens[i++];
				Token t3 = tokens[i++];
				Token t4 = tokens[i++];

				output.push_back(0x01);
				output.push_back(0x00);

				if (t2.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается регистр!", t2.value.c_str());
					ok = 0;
					continue;
				}

				if (t3.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t3.line,t3.offset, "Ожидается регистр!", t3.value.c_str());
					ok = 0;
					continue;
				}

				if (t4.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t4.line,t4.offset, "Ожидается регистр!", t4.value.c_str());
					ok = 0;
					continue;
				}

				output.push_back(register_id(t2.value));
				output.push_back(register_id(t3.value));
				output.push_back(register_id(t4.value));
			}

			if (t1.value == "sub") {
				Token t2 = tokens[i++];
				Token t3 = tokens[i++];
				Token t4 = tokens[i++];

				output.push_back(0x01);
				output.push_back(0x01);

				if (t2.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается регистр!", t2.value.c_str());
					ok = 0;
					continue;
				}

				if (t3.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t3.line,t3.offset, "Ожидается регистр!", t3.value.c_str());
					ok = 0;
					continue;
				}

				if (t4.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t4.line,t4.offset, "Ожидается регистр!", t4.value.c_str());
					ok = 0;
					continue;
				}

				output.push_back(register_id(t2.value));
				output.push_back(register_id(t3.value));
				output.push_back(register_id(t4.value));
			}

			if (t1.value == "cmp") {
				Token t2 = tokens[i++];
				Token t3 = tokens[i++];

				output.push_back(0x01);
				output.push_back(0x00);

				if (t2.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается регистр!", t2.value.c_str());
					ok = 0;
					continue;
				}

				if (t3.type != reg) {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t3.line,t3.offset, "Ожидается регистр!", t3.value.c_str());
					ok = 0;
					continue;
				}

				output.push_back(0);
				output.push_back(register_id(t2.value));
				output.push_back(register_id(t3.value));
			}

			if (t1.value == "jmp") {
				Token t2 = tokens[i++];

				output.push_back(0x00);
				output.push_back(0x01);
				output.push_back(register_id("l31"));

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "je") {
				Token t2 = tokens[i++];

				output.push_back(0x03);
				output.push_back(0x00);
				output.push_back(0);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "jne") {
				Token t2 = tokens[i++];

				output.push_back(0x03);
				output.push_back(0x01);
				output.push_back(0);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "jl") {
				Token t2 = tokens[i++];

				output.push_back(0x03);
				output.push_back(0x02);
				output.push_back(0);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "jb") {
				Token t2 = tokens[i++];

				output.push_back(0x03);
				output.push_back(0x03);
				output.push_back(0);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "jle") {
				Token t2 = tokens[i++];

				output.push_back(0x03);
				output.push_back(0x04);
				output.push_back(0);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}

			if (t1.value == "jbe") {
				Token t2 = tokens[i++];

				output.push_back(0x03);
				output.push_back(0x05);
				output.push_back(0);

				if (t2.type == number) {
					uint128 n = string_to_number128(t2.value) + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else if (t2.type == hex_number) {
					uint128 n = hex_string_to_number128(t2.value); + offset;
					for (int i = 0; i < 8; i++) {
						output.push_back((n >> i * 8) & 0xff);
					}
				} else {
					printf("%i,%i \e[1;31m%-6s\e[m %s\n",
						t2.line,t2.offset, "Ожидается число!", t2.value.c_str());
					ok = 0;
					continue;
				}
			}
		}

		return ok;
	}
};
