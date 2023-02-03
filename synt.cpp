#include <synt.h>


typedef unsigned __int128 uint128;


uint128 pow(uint128 v, uint128 p) {
	uint128 res = 1;

	for (int i = 0; i < p; i++)
		res *= v;

	return res;
}


uint128 s2r(std::string val) {
	uint128 res = 0;

	for (int i = 0; i < val.size(); i++)
		res += (val[i] - '0') * pow(10, (val.size() - i - 1));

	return res;
}

uint128 sh2r(std::string val) {
	uint128 res = 0;

	for (int i = 2; i < val.size(); i++)
		if (val[i] <= '9')
			res += (val[i] - '0') * pow(16, (val.size() - i - 1));
		else
			res += (val[i] - 'a' + 10) * pow(16, (val.size() - i - 1));

	return res;
}


template<class T> char val_in_vector(std::vector<T> vec, T val) {
	for (int i = 0; i < vec.size(); i++) {
		if (vec[i] == val) return 1;
	}

	return 0;
}


char register_size(std::string val) {
	if (val[0] == 'r') return 16;
	if (val[0] == 'l') return 8;
	if (val[0] == 'i') return 4;
	if (val[0] == 's') return 2;
	if (val[0] == 'b') return 1;
	return 0;
}

char register_id(std::string val) {
	char c = val[0];

	val.erase(0,1);

	char res = s2r(val) * 5;

	if (c == 'r') return res + 0;
	if (c == 'l') return res + 1;
	if (c == 'i') return res + 2;
	if (c == 's') return res + 3;
	if (c == 'b') return res + 4;

	return -1;
}


void Synt::clear() {
	tokens.clear();
	output.clear();
	labels.clear();
}


char Synt::parse() {
	char ok = 1;

	int i = 0;

	while (i < tokens.size()) {
		Token t1 = tokens[i++];

		if (t1.type == TYPE_PREPROCESSOR) {
			if (t1.value == "%org") {
				Token t2 = tokens[i++];

				if (t2.type == TYPE_NUMBER) {
					org = s2r(t2.value);
				} else if (t2.type == TYPE_HEX_NUMBER) {
					org = sh2r(t2.value);
				}
			}
		} else if (t1.type == TYPE_INSTRUCTION) {
			if (t1.value == "hlt") {
				output.push_back(0x00);
				output.push_back(0x00);
				output.push_back(0x00);
			} else

			if (t1.value == "mov") {
				Token t2 = tokens[i++];

				if (t2.type == TYPE_REGISTER) {
					Token t3 = tokens[i++];

					if (t3.type == TYPE_NUMBER) {
						output.push_back(0x01);
						output.push_back(0x00);
						output.push_back(register_id(t2.value));

						uint128 num = s2r(t3.value);

						for (int i = 0; i < register_size(t2.value); i++) {
							output.push_back((num >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_HEX_NUMBER) {
						output.push_back(0x01);
						output.push_back(0x00);
						output.push_back(register_id(t2.value));

						uint128 num = sh2r(t3.value);

						for (int i = 0; i < register_size(t2.value); i++) {
							output.push_back((num >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_REGISTER) {
						output.push_back(0x01);
						output.push_back(0x01);
						output.push_back(register_id(t2.value));
						output.push_back(register_id(t3.value));
					} else if (t3.type == TYPE_ADDR_READ) {
						uint128 addr = s2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x02);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_HEX_ADDR_READ) {
						uint128 addr = sh2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x02);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_ADDR_WRITE) {
						uint128 addr = s2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x03);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_HEX_ADDR_WRITE) {
						uint128 addr = sh2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x03);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_ADDR_READ_ABS) {
						uint128 addr = s2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x04);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_HEX_ADDR_READ_ABS) {
						uint128 addr = sh2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x04);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_ADDR_WRITE_ABS) {
						uint128 addr = s2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x05);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_HEX_ADDR_WRITE_ABS) {
						uint128 addr = sh2r(t3.value.substr(1,t3.value.size()-2)) + org;

						output.push_back(0x01);
						output.push_back(0x05);
						output.push_back(register_id(t2.value));

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else if (t3.type == TYPE_UNDEFINED) {
						output.push_back(0x01);
						output.push_back(0x00);
						output.push_back(register_id(t2.value));

						uint128 addr = 0;

						if (t3.value == "$") {
							addr = output.size() - 3 + org;
						} else {
							Label label;

							label.value = t3.value;
							label.offset = output.size();

							alabels.push_back(label);
						}

						for (int i = 0; i < 8; i++) {
							output.push_back((addr >> (i << 3)) & 0xff);
						}
					} else {
						printf("%i,%i: \e[1;31m%s\e[m\n", t3.line,t3.offset, "Ожидается число/регистр/адрес!");
						ok = 0;
					}
				} else {
					printf("%i,%i: \e[1;31m%s\e[m\n", t2.line,t2.offset, "Ожидается регистр!");
					ok = 0;
				}
			}

			if (t1.value == "jmp") {
				Token t2 = tokens[i++];

				output.push_back(0x01);
				output.push_back(0x00);
				output.push_back(0xfb);

				if (t2.type == TYPE_NUMBER) {
					uint128 addr = s2r(t2.value) + org;

					for (int i = 0; i < 8; i++) {
						output.push_back((addr >> (i << 3)) & 0xff);
					}
				} else if (t2.type == TYPE_UNDEFINED) {
					uint128 addr = 0;

					if (t2.value == "$") {
						addr = output.size() - 3 + org;
					} else {
						Label label;

						label.value = t2.value;
						label.offset = output.size();

						alabels.push_back(label);
					}

					for (int i = 0; i < 8; i++) {
						output.push_back((addr >> (i << 3)) & 0xff);
					}
				}
			}
		} else if (t1.type == TYPE_LABEL) {
			char local_ok = 1;
			for (int i = 0; i < labels.size(); i++) {
				if (labels[i].value == t1.value) {
					printf("%i,%i: \e[1;31m%s\e[m\n", t1.line,t1.offset, "Метка с таким названием уже существует!");
					ok = 0;
					local_ok = 0;
				}
			}

			if (local_ok == 1) {
				Label label;

				label.value = t1.value;
				label.offset = output.size() + org;

				labels.push_back(label);
			}
		} else {
			printf("%i,%i: \e[1;31m%s\e[m\n", t1.line,t1.offset, "Ожидается препроцессор/инструкция/метка!");
			ok = 0;
		}
	}

	return ok;
}


char Synt::parse_labels() {
	char ok = 1;

	for (int i = 0; i < alabels.size(); i++) {
		int lid = -1;

		for (int j = 0; j < labels.size(); j++) {
			if (labels[j].value == alabels[i].value + ":")
				lid = j;
		}

		if (lid == -1) {
			printf("%i,%i: \e[1;31m%s\e[m%s\e[1;31m%s\e[m\n", 0,0, "Метка ", "", " не найдена!");
			ok = 0;
		} else {
			for (int j = 0; j < 8; j++) {
				output[alabels[i].offset + j] = (labels[lid].offset >> (j << 3)) & 0xff;
			}
		}
	}

	return ok;
}
