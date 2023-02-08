#ifndef SYNT_H
#define SYNT_H

#include <lex.h>
#include <elf.h>

typedef struct {
	char ok;
	char *code;
	unsigned int code_size;
	Name_sec_elem *names;
	unsigned int names_count;
	Addr_sec_elem *addrs;
	unsigned int addrs_count;
} Synt_result;

Synt_result synt_parse(Lex_result lex);

#endif // SYNT_H
