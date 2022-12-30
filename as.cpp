#include <cstdio>
#include <getopt.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <lex.cpp>
#include <synt.cpp>


#ifndef DEBUG
	#define LOG
#else
	#define LOG(...) printf(__VA_ARGS__)
#endif


void print_help();


char *output_filename = (char*)"a.out";
char *input_filename;
char *input_data;

Lex lex;
Synt synt;


int main(int argc, char **argv) {
	if (argc == 1) {
		print_help();
		return 0;
	}


	int opt;

	do {
		switch(opt = getopt(argc, argv, "ho:")) {
			case 'h':
				print_help();
				return 0;

			case 'o':
				output_filename = argv[optind];
				break;

			case -1:
				input_filename = argv[optind];
				break;

			default:
				print_help();
				return 0;
		}
	} while (opt != -1);


	if (input_filename == NULL) {
		print_help();
		return 0;
	}


	FILE* input_file = fopen(input_filename, "r");

	if (input_file == NULL) {
		fclose(input_file);
		printf("Файла %s не существует!\n", input_filename);
		return 2;
	}

	fseek(input_file, 0, SEEK_END);
	long input_size = ftell(input_file);
	rewind(input_file);

	input_data = new char[input_size];

	fread(input_data, 1, input_size, input_file);

	fclose(input_file);


	if (lex.parse(input_data, input_size) == 0) {
		return -1;
	}



	/*for (int i = 0; i < lex.tokens.size(); i++) {
		printf("%s %i\n", lex.tokens[i].value.c_str(), lex.tokens[i].type);
	}*/


	if (synt.parse(lex.tokens) == 0) {
		return -1;
	}


	FILE *output_file = fopen(output_filename, "wb");
	for (int i = 0; i < synt.output.size(); i++) {
		fputc(synt.output[i], output_file);
	}
	fclose(output_file);
}


void print_help() {
	printf("Компилятор ассемблера для архитектуры ???\n\tИспользование:\n\t\tas <input file> -o <output file>\n\t-o <file path>\n\t\tПуть к выходному файлу.\n");
}
