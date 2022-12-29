#pragma once

#include <types.cpp>


bool is_number(std::string s) {
	for (int i = 0; i < s.size(); i++)
		if ('9' < s[i] || s[i] < '0') return 0;
	return 1;
}


bool is_register(std::string s) {
	if (s[0] != 'r' && s[0] != 'l' && s[0] != 'i' && s[0] != 's' && s[0] != 'b')
		return 0;

	std::string s2 = s.substr(1,s.size()-1);

	return is_number(s2);
}


template <class T>
bool is_value_in_vector(T val, std::vector<T> vec) {
	for (int i = 0; i < vec.size(); i++)
		if (val == vec[i]) return 1;
	return 0;
}

template <class T>
T pow(T a, T b) {
	T r = 1;

	for (int i = 0; i < b; i++) {
		r *= a;
	}

	return r;
}

uint8 string_to_number8(std::string s) {
	uint8 res = 0;

	for (int i = 0; i < s.size(); i++) {
		res += (s[i] - '0') * pow(10,(char)s.size()-i-1);
	}

	return res;
}

uint128 string_to_number128(std::string s) {
	uint128 res = 0;

	for (int i = 0; i < s.size(); i++) {
		res += (s[i] - '0') * pow((uint128)10,(uint128)s.size()-i-1);
	}

	return res;
}

unsigned int register_id(std::string s) {
	std::string s2 = s.substr(1,s.size()-1);

	unsigned int res = string_to_number8(s2) * 5;

	//printf("\n%s %i\n\n", s2.c_str(), res);

	if (s[0] == 'r')
		res += 0;

	if (s[0] == 'l')
		res += 1;

	if (s[0] == 'i')
		res += 2;

	if (s[0] == 's')
		res += 3;

	if (s[0] == 'b')
		res += 4;

	return res;
}

unsigned int register_size(std::string s) {
	if (s[0] == 'r')
		return 16;
	if (s[0] == 'l')
		return 8;
	if (s[0] == 'i')
		return 4;
	if (s[0] == 's')
		return 2;
	if (s[0] == 'b')
		return 1;
	return -1;
}
