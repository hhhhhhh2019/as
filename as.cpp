#include <stdio.h>
#include <string.h>
#include <lex.h>
#include <synt.h>



#ifndef DEBUG
	#define LOG
#else
	#define LOG(...) printf(__VA_ARGS__)
#endif

#define INFO(msg) LOG("\e[1;34m%s\e[m", msg)


char* input_name;
char* output_name = (char*)"a.out";
char ok = 0;

char* input;


Lex lex;
Synt synt;


void print_help();


int main(int argc, char** argv) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_help();
			return 0;
		} else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
			input_name = argv[++i];
			ok = 1;
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
			output_name = argv[++i];
		} else {
			printf("Неизвестный аргумент! %s -h для справки.", argv[0]);
		}
	}

	if (ok == 0) {
		printf("аргумент -i обязателен!\n");
		return 1;
	}


	FILE* f = fopen(input_name, "r");

	if (f == NULL) {
		printf("Файл %s не найден!\n", input_name);
		return 2;
	}

	fseek(f, 0L, SEEK_END);
	unsigned int input_size = ftell(f);
	rewind(f);

	input = new char[input_size + 1];

	fread(input, 1, input_size, f);

	input[input_size] = 0;

	fclose(f);


	lex.clear();
	lex.code = input;
	lex.code_length = input_size;

	lex.parse();


	synt.clear();
	synt.tokens = lex.tokens;

	if ((synt.parse() & synt.parse_labels()) == 0) {
		return 1;
	}


	f = fopen(output_name, "wb");

	for (int i = 0; i < synt.output.size(); i++) {
		putc(synt.output[i], f);
	}

	fclose(f);
}


void print_help() {
	printf("Ассемблер для ??? архитектуры.\n\t-h --help\tвывод этого сообщения.\n\t-i --input\tпуть к входному файлу.\n\t-o --output\tпуть к выходному файлу.\n");
}
