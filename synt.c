#include <synt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>

Synt_result synt_parse(Lex_result lex) {
	Synt_result res;

	res.code = malloc(0);
	res.names = malloc(0);
	res.addrs = malloc(0);

	res.code_size = 0;
	res.names_count = 0;
	res.addrs_count = 0;

	res.ok = 1;

	for (int i = 0; i < lex.count;) {
		Token t1 = lex.tokens[i++];

		if (t1.type == TYPE_INSTRUCTION) {
			if (strcmp(t1.value, "nop\0") == 0) {
				res.code = realloc(res.code, ++res.code_size);
				res.code[res.code_size - 1] = 0x00;
			} else if (strcmp(t1.value, "hlt\0") == 0) {
				res.code = realloc(res.code, ++res.code_size);
				res.code[res.code_size - 1] = 0x01;
			} else if (strcmp(t1.value, "movb\0") == 0) {
				Token t2 = lex.tokens[i++];

				if (t2.type == TYPE_REGISTER) { // movb n
					Token t3 = lex.tokens[i++];

					if (t3.type == TYPE_NUMBER) {
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x02;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(t3.value);
					} else if (t3.type == TYPE_HEX_NUMBER) { // movb n
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x02;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = h2n(t3.value);
					}  else if (t3.type == TYPE_REGISTER) { // movb r
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x07;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(t3.value+1);
					} else if (t3.type == TYPE_ADDR_START) { // movb from RAM
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_ADDR_END) {
							if (t.type == TYPE_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"[\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x0c;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else if (t3.type == TYPE_GLOBAL_ADDR_START) { // movb from RAMg
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_GLOBAL_ADDR_END) {
							if (t.type == TYPE_GLOBAL_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x16;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/число!\n", t3.line,t3.offset, t3.value);
						res.ok = 0;
					}
				} else if (t2.type == TYPE_ADDR_START) { // movb to RAM
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_ADDR_END) {
						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x11;
					res.code[res.code_size - 9] = s2n(t3.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else if (t2.type == TYPE_GLOBAL_ADDR_START) { // movb to RAMg
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_GLOBAL_ADDR_END) {
						if (t.type == TYPE_GLOBAL_ADDR_START) {
							printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
							res.ok = 0;
							break;
						}

						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x1b;
					res.code[res.code_size - 9] = s2n(t2.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else {
					printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/адрес!\n", t2.line,t2.offset, t2.value);
					res.ok = 0;
				}
			} else if (strcmp(t1.value, "movs\0") == 0) {
				Token t2 = lex.tokens[i++];

				if (t2.type == TYPE_REGISTER) {
					Token t3 = lex.tokens[i++];

					if (t3.type == TYPE_NUMBER) { // movs n
						res.code_size += 4;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 4] = 0x03;
						res.code[res.code_size - 3] = s2n(t2.value+1);

						__int128 num = s2n(t3.value);

						for (int i = 0; i < 2; i++)
							res.code[res.code_size - 2 + i] = (num >> (i << 3)) & 0xff;
					} else if (t3.type == TYPE_HEX_NUMBER) { // movs n
						res.code_size += 4;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 4] = 0x03;
						res.code[res.code_size - 3] = s2n(t2.value+1);

						__int128 num = h2n(t3.value);

						for (int i = 0; i < 2; i++)
							res.code[res.code_size - 2 + i] = (num >> (i << 3)) & 0xff;
					}  else if (t3.type == TYPE_REGISTER) { // movs r
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x08;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(t3.value+1);
					} else if (t3.type == TYPE_ADDR_START) { // movs from RAM
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_ADDR_END) {
							if (t.type == TYPE_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"[\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x0d;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else if (t3.type == TYPE_GLOBAL_ADDR_START) { // movs from RAMg
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_GLOBAL_ADDR_END) {
							if (t.type == TYPE_GLOBAL_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x17;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/число!\n", t3.line,t3.offset, t3.value);
						res.ok = 0;
					}
				} else if (t2.type == TYPE_ADDR_START) { // movs to RAM
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_ADDR_END) {
						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x12;
					res.code[res.code_size - 9] = s2n(t3.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else if (t2.type == TYPE_GLOBAL_ADDR_START) { // movs to RAMg
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_GLOBAL_ADDR_END) {
						if (t.type == TYPE_GLOBAL_ADDR_START) {
							printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
							res.ok = 0;
							break;
						}

						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x1c;
					res.code[res.code_size - 9] = s2n(t2.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else {
					printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/адрес!\n", t2.line,t2.offset, t2.value);
					res.ok = 0;
				}
			} else if (strcmp(t1.value, "movi\0") == 0) {
				Token t2 = lex.tokens[i++];

				if (t2.type == TYPE_REGISTER) {
					Token t3 = lex.tokens[i++];

					if (t3.type == TYPE_NUMBER) { // movi n
						res.code_size += 6;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 6] = 0x04;
						res.code[res.code_size - 5] = s2n(t2.value+1);

						__int128 num = s2n(t3.value);

						for (int i = 0; i < 4; i++)
							res.code[res.code_size - 4 + i] = (num >> (i << 3)) & 0xff;
					} else if (t3.type == TYPE_HEX_NUMBER) { // movi n
						res.code_size += 6;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 6] = 0x04;
						res.code[res.code_size - 5] = s2n(t2.value+1);

						__int128 num = h2n(t3.value);

						for (int i = 0; i < 4; i++)
							res.code[res.code_size - 4 + i] = (num >> (i << 3)) & 0xff;
					}  else if (t3.type == TYPE_REGISTER) { // movi r
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x09;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(t3.value+1);
					} else if (t3.type == TYPE_ADDR_START) { // movi from RAM
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_ADDR_END) {
							if (t.type == TYPE_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"[\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x0e;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else if (t3.type == TYPE_GLOBAL_ADDR_START) { // movi from RAMg
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_GLOBAL_ADDR_END) {
							if (t.type == TYPE_GLOBAL_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x18;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/число!\n", t3.line,t3.offset, t3.value);
						res.ok = 0;
					}
				} else if (t2.type == TYPE_ADDR_START) { // movi to RAM
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_ADDR_END) {
						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x13;
					res.code[res.code_size - 9] = s2n(t3.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else if (t2.type == TYPE_GLOBAL_ADDR_START) { // movi to RAMg
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_GLOBAL_ADDR_END) {
						if (t.type == TYPE_GLOBAL_ADDR_START) {
							printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
							res.ok = 0;
							break;
						}

						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x1d;
					res.code[res.code_size - 9] = s2n(t2.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else {
					printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/адрес!\n", t2.line,t2.offset, t2.value);
					res.ok = 0;
				}
			} else if (strcmp(t1.value, "movl\0") == 0) {
				Token t2 = lex.tokens[i++];

				if (t2.type == TYPE_REGISTER) {
					Token t3 = lex.tokens[i++];

					if (t3.type == TYPE_NUMBER) { // movl n
						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size -10] = 0x05;
						res.code[res.code_size - 9] = s2n(t2.value+1);

						__int128 num = s2n(t3.value);

						for (int i = 0; i < 8; i++)
							res.code[res.code_size - 8 + i] = (num >> (i << 3)) & 0xff;
					} else if (t3.type == TYPE_HEX_NUMBER) { // movl n
						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size -10] = 0x05;
						res.code[res.code_size - 9] = s2n(t2.value+1);

						__int128 num = h2n(t3.value);

						for (int i = 0; i < 8; i++)
							res.code[res.code_size - 8 + i] = (num >> (i << 3)) & 0xff;
					}  else if (t3.type == TYPE_REGISTER) { // movl r
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x0a;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(t3.value+1);
					} else if (t3.type == TYPE_ADDR_START) { // movl from RAM
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_ADDR_END) {
							if (t.type == TYPE_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"[\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x0f;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else if (t3.type == TYPE_GLOBAL_ADDR_START) { // movl from RAMg
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_GLOBAL_ADDR_END) {
							if (t.type == TYPE_GLOBAL_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x19;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/число!\n", t3.line,t3.offset, t3.value);
						res.ok = 0;
					}
				} else if (t2.type == TYPE_ADDR_START) { // movl to RAM
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_ADDR_END) {
						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x14;
					res.code[res.code_size - 9] = s2n(t3.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else if (t2.type == TYPE_GLOBAL_ADDR_START) {
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_GLOBAL_ADDR_END) { // movl to RAMg
						if (t.type == TYPE_GLOBAL_ADDR_START) {
							printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
							res.ok = 0;
							break;
						}

						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x1e;
					res.code[res.code_size - 9] = s2n(t2.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else {
					printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/адрес!\n", t2.line,t2.offset, t2.value);
					res.ok = 0;
				}
			} else if (strcmp(t1.value, "movr\0") == 0) {
				Token t2 = lex.tokens[i++];

				if (t2.type == TYPE_REGISTER) {
					Token t3 = lex.tokens[i++];

					if (t3.type == TYPE_NUMBER) { // movr n
						res.code_size += 18;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 18] = 0x06;
						res.code[res.code_size - 17] = s2n(t2.value+1);

						__int128 num = s2n(t3.value);

						for (int i = 0; i < 16; i++)
							res.code[res.code_size - 16 + i] = (num >> (i << 3)) & 0xff;
					} else if (t3.type == TYPE_HEX_NUMBER) { // movr n
						res.code_size += 18;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 18] = 0x06;
						res.code[res.code_size - 17] = s2n(t2.value+1);

						__int128 num = h2n(t3.value);

						for (int i = 0; i < 16; i++)
							res.code[res.code_size - 16 + i] = (num >> (i << 3)) & 0xff;
					}  else if (t3.type == TYPE_REGISTER) { // movr r
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x0b;
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(t3.value+1);
					} else if (t3.type == TYPE_ADDR_START) { // movr from RAM
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_ADDR_END) {
							if (t.type == TYPE_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"[\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x10;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else if (t3.type == TYPE_GLOBAL_ADDR_START) { // movr from RAMg
						Token t = lex.tokens[i++];
						char* exp = calloc(64,1);
						unsigned int exp_size = 0;

						while (t.type != TYPE_GLOBAL_ADDR_END) {
							if (t.type == TYPE_GLOBAL_ADDR_START) {
								printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
								res.ok = 0;
								break;
							}

							memcpy(&exp[exp_size], t.value, strlen(t.value));
							exp_size += strlen(t.value);
							t = lex.tokens[i++];
						}

						res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
						memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
						res.addrs[res.addrs_count-1].offset = res.code_size + 2;
						free(exp);

						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 10] = 0x1a;
						res.code[res.code_size - 9] = s2n(t2.value+1);
						for (int j = 0; j < 8; j++)
							res.code[res.code_size - 8 + j] = 0;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/число!\n", t3.line,t3.offset, t3.value);
						res.ok = 0;
					}
				} else if (t2.type == TYPE_ADDR_START) { // movr to RAM
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_ADDR_END) {
						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x15;
					res.code[res.code_size - 9] = s2n(t3.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else if (t2.type == TYPE_GLOBAL_ADDR_START) {
					Token t = lex.tokens[i++];
					char* exp = calloc(64,1);
					unsigned int exp_size = 0;

					while (t.type != TYPE_GLOBAL_ADDR_END) { // movr to RAMg
						if (t.type == TYPE_GLOBAL_ADDR_START) {
							printf("\e[1;31mОшибка!\e[m %d,%d: знак \"{\" не ожидался!\n", t.line,t.offset, t.value);
							res.ok = 0;
							break;
						}

						memcpy(&exp[exp_size], t.value, strlen(t.value));
						exp_size += strlen(t.value);
						t = lex.tokens[i++];
					}

					res.addrs = realloc(res.addrs, (++res.addrs_count)*sizeof(Addr_sec_elem));
					memcpy(res.addrs[res.addrs_count-1].name, exp, exp_size);
					res.addrs[res.addrs_count-1].offset = res.code_size + 2;
					free(exp);

					Token t3 = lex.tokens[i++];

					res.code_size += 10;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size - 10] = 0x1f;
					res.code[res.code_size - 9] = s2n(t2.value+1);
					for (int j = 0; j < 8; j++)
						res.code[res.code_size - 8 + j] = 0;
				} else {
					printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается регистр/адрес!\n", t2.line,t2.offset, t2.value);
					res.ok = 0;
				}
			}

		} else {
			printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается инструкция/препрцессор/метка!\n", t1.line,t1.offset, t1.value);
			res.ok = 0;
		}
	}

	return res;
}
