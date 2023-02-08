#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <lex.h>
#include <synt.h>


char* input_path = NULL;
char* output_path = "a.out\0";

char* input_data;
unsigned int input_size = 0;


void print_help();


int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_help();
			return 0;
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
			output_path = argv[++i];
		} else {
			input_path = argv[i];
		}
	}

	if (input_path == NULL) {
		print_help();
		printf("Исользование: %s <filename> -o <filename>\n", argv[0]);
		return 1;
	}


	FILE* f = fopen(input_path, "r");

	if (f == NULL) {
		printf("Файл %s не найден!\n", input_path);
		return 2;
	}

	fseek(f, 0, SEEK_END);
	input_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	input_data = malloc(input_size + 1);

	fread(input_data, 1, input_size, f);

	input_data[input_size] = 0;


	Lex_result lex = lex_parse(input_data, input_size);
	Synt_result synt = synt_parse(lex);

	printf("Токены:\n");

	for (int i = 0; i < lex.count; i++) {
		printf("%d,%d %d %s\n", lex.tokens[i].line, lex.tokens[i].offset, lex.tokens[i].type, lex.tokens[i].value);

		free(lex.tokens[i].value);
	}

	printf("\nКод:\n");

	for (int i = 0; i < synt.code_size; i++) {
		printf("%02x ", synt.code[i] & 0xff);
	}

	printf("\nАдреса:\n");

	for (int i = 0; i < synt.addrs_count; i++) {
		putc('\t', stdout);
		for (int j = 0; j < 64; j++)
			if (synt.addrs[i].name[j] == 0)
				break;
			else
				putc(synt.addrs[i].name[j], stdout);
		printf(": %d\n", synt.addrs[i].offset);
	}

	free(synt.code);
	free(synt.names);
	free(synt.addrs);

	free(input_data);
	free(lex.tokens);
}


void print_help() {
	printf("Компилятор ассемблера для архитектуы ???.\n\t-h --help\tвыод этого сообщения.\n\t-o --output\tпуть к итоговому файлу.\n");
}
