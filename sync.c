#include <synt.h>
#include <utils.h>
#include <math_parser.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


unsigned int ti = 0;


char match(Token token, char types[], int count) {
	for (int i = 0; i < count; i++)
		if (token.type == types[i])
			return 1;

	printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидается ", token.line, token.offset, token.value);

	for (int i = 0; i < count - 1; i++) {
		print_token_type(types[i]);
		putc('/', stdout);
	}

	print_token_type(types[count - 1]);
	putc('\n', stdout);

	return 0;
}


void next_line(Token* tokens) {
	Token t = tokens[ti++];
	while (t.type != TYPE_NEW_LINE)
		t = tokens[ti++];
	ti++;
}


char expect_new_line(Token* tokens) {
	char ok = 1;
	Token t = tokens[ti++];
	while (t.type != TYPE_NEW_LINE) {
		//printf("%s\n", t.value);
		if (t.type != TYPE_NEW_LINE) {
			if (ok == 1) {
				printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", t.line,t.offset);
				ok = 0;
			}
			printf("%s ", t.value);
		}
		t = tokens[ti++];
	}
	ti++;
	if (ok == 0)
		putc('\n', stdout);
	return ok;
}


Token* get_exp(Token* tokens, char end_type, unsigned int *count, char *has_name) {
	Token* res = malloc(0);
	Token t = tokens[ti++];

	while (t.type != end_type && t.type != TYPE_NEW_LINE) {
		if (t.type == TYPE_UNDEFINED)
			*has_name = 1;

		res = realloc(res, ++(*count) * sizeof(Token));
		memcpy(&res[*count - 1], &t, sizeof(Token));

		t = tokens[ti++];
	}

	return res;
}


Synt_result synt_parse(Lex_result lex) {
	Synt_result res;

	res.code = malloc(0);
	res.code_size = 0;
	res.addrs = malloc(0);
	res.addrs_count = 0;
	res.names = malloc(0);
	res.names_count = 0;

	ti = 0;

	while (ti < lex.count) {
		Token t1 = lex.tokens[ti++];

		if (t1.type == TYPE_NEW_LINE)
			continue;

		if (match(t1, (char[]){TYPE_INSTRUCTION}, 1) == 0) {
			next_line(lex.tokens);
			continue;
		}

		if (t1.type == TYPE_INSTRUCTION) {
			if (strcmp(t1.value, "movb\0") == 0) {
				Token t2 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER,TYPE_ADDR_START,TYPE_GLOBAL_ADDR_START}, 3) == 0) {
					next_line(lex.tokens);
					continue;
				}

				if (t2.type == TYPE_REGISTER) {
					unsigned int count = 0;
					char has_name = 0;

					Token* tokens = get_exp(lex.tokens, TYPE_NEW_LINE, &count, &has_name);

					if (count == 1 && tokens[0].type == TYPE_REGISTER) {
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x07; // movb r
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = s2n(tokens[0].value+1);
					} else if (tokens[0].type == TYPE_ADDR_START || tokens[0].type == TYPE_GLOBAL_ADDR_START) {
						char has_reg = 0;
						unsigned int end_id = 0;

						for (int i = 0; i < count; i++) {
							if (tokens[i].type == TYPE_REGISTER)
								has_reg = 1;
							if (tokens[i].type == tokens[0].type + 1)
								end_id = i;
						}

						if (has_reg == 1) {
							if (count != 3) {
								if (end_id != 2) {
									printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор в выражении!: ", tokens[2].line,tokens[2].offset);
									for (int i = 2; i < count-1; i++)
										printf("%s ", tokens[i].value);
									putc('\n', stdout);
								} else {
									printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[3].line,tokens[3].offset);
									for (int i = 3; i < count; i++)
										printf("%s ", tokens[i].value);
									putc('\n', stdout);
								}
								res.ok = 0;
								next_line(lex.tokens);
								free(tokens);
								continue;
							} else {
								res.code_size += 3;
								res.code = realloc(res.code, res.code_size);
								res.code[res.code_size - 3] = 0x20; // movb from RAMr
								res.code[res.code_size - 2] = s2n(t2.value+1);
								res.code[res.code_size - 1] = s2n(tokens[1].value+1);
							}
						} else {
							res.code_size += 10;
							res.code = realloc(res.code, res.code_size);
							if (tokens[0].type == TYPE_ADDR_START)
								res.code[res.code_size -10] = 0x0c; // movb from RAM
							else
								res.code[res.code_size -10] = 0x16; // movb from RAMg
							res.code[res.code_size - 9] = s2n(t2.value+1);
							res.code[res.code_size - 8] = 0;
							res.code[res.code_size - 7] = 0;
							res.code[res.code_size - 6] = 0;
							res.code[res.code_size - 5] = 0;
							res.code[res.code_size - 4] = 0;
							res.code[res.code_size - 3] = 0;
							res.code[res.code_size - 2] = 0;
							res.code[res.code_size - 1] = 0;

							Node* root = create_math_tree(&tokens[1], end_id-1);
							unsigned int size = get_tree_size(root);

							if (size != end_id-1) {
								printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор в выражении!: ", tokens[size].line,tokens[size].offset);
								for (int i = size+1; i < count-1; i++)
									printf("%s ", tokens[i].value);
								putc('\n', stdout);
								res.ok = 0;
								next_line(lex.tokens);
								free(tokens);
								clear_node(root);
								continue;
							}

							if (count > end_id + 1) {
								printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[size+2].line,tokens[size+2].offset);
								for (int i = size+2; i < count; i++)
									printf("%s ", tokens[i].value);
								putc('\n', stdout);
								res.ok = 0;
								next_line(lex.tokens);
								free(tokens);
								clear_node(root);
								continue;
							}

							if (has_name == 0) {
									__int128 num = get_result(root);
									for (int i = 0; i < 8; i++)
										res.code[res.code_size - 8 + i] = (num >> (i << 3)) & 0xff;
							} else {
								char* exp = calloc(64,1);
								unsigned int exp_size = 0;

								for (int i = 1; i < count - 1; i++) {
									unsigned int len = strlen(tokens[i].value);
									memcpy(&exp[exp_size], tokens[i].value, len);
									exp_size += len;
								}

								res.addrs = realloc(res.addrs, ++res.addrs_count * sizeof(Addr_sec_elem));
								Addr_sec_elem* addr = calloc(sizeof(Addr_sec_elem), 1);
								memcpy(addr->name, exp, exp_size);
								addr->offset = res.code_size - 8;
								memcpy(&res.addrs[res.addrs_count-1], addr, sizeof(Addr_sec_elem));
								free(addr);
								free(exp);
							}

							clear_node(root);
						}
					} else {
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x02; // movb n
						res.code[res.code_size - 2] = s2n(t2.value+1);
						res.code[res.code_size - 1] = 0;

						if (has_name == 0) {
							Node* root = create_math_tree(tokens, count);
							unsigned int size = get_tree_size(root);
							if (size == count) {
								__int128 num = get_result(root);
								res.code[res.code_size - 1] = num & 0xff;
							} else
								ti -= count - size + 1;
							clear_node(root);
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Использование ссылок вместе с \"movb\" не допускается!\n", tokens[0].line,tokens[0].offset);
							res.ok = 0;
							next_line(lex.tokens);
							free(tokens);
							continue;
						}
					}

					free(tokens);
				} else if (t2.type == TYPE_ADDR_START || t2.type == TYPE_GLOBAL_ADDR_START) {
					unsigned int count = 0;
					char has_name = 0;
					char has_reg = 0;

					Token* tokens = get_exp(lex.tokens, t2.type+1, &count, &has_name);

					for (int i = 0; i < count; i++)
						if (tokens[i].type == TYPE_REGISTER)
							has_reg = 1;

					Token t3 = lex.tokens[ti++];

					if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
						next_line(lex.tokens);
						free(tokens);
						continue;
					}

					if (has_reg == 1) {
						if (count == 1) {
							res.code_size += 3;
							res.code = realloc(res.code, res.code_size);
							res.code[res.code_size - 3] = 0x25; // movb to RAMr
							res.code[res.code_size - 2] = s2n(t3.value+1);
							res.code[res.code_size - 1] = s2n(tokens[0].value+1);
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор в выражении!: ", tokens[1].line,tokens[1].offset);
							for (int i = 1; i < count; i++)
								printf("%s ", tokens[i].value);
							putc('\n', stdout);
							res.ok = 0;
							next_line(lex.tokens);
							free(tokens);
							continue;
						}
					} else {
						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						if (t2.type == TYPE_ADDR_START)
							res.code[res.code_size -10] = 0x11; // movb to RAM
						else
							res.code[res.code_size -10] = 0x1b; // movb to RAMg
						res.code[res.code_size - 9] = s2n(t3.value+1);
						res.code[res.code_size - 8] = 0;
						res.code[res.code_size - 7] = 0;
						res.code[res.code_size - 6] = 0;
						res.code[res.code_size - 5] = 0;
						res.code[res.code_size - 4] = 0;
						res.code[res.code_size - 3] = 0;
						res.code[res.code_size - 2] = 0;
						res.code[res.code_size - 1] = 0;

						Node* root = create_math_tree(tokens, count);
						unsigned int size = get_tree_size(root);
						if (size != count) {
							printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор в выражении!: ", tokens[size].line,tokens[size].offset);
							for (int i = size; i < count; i++)
								printf("%s ", tokens[i].value);
							putc('\n', stdout);
							res.ok = 0;
							next_line(lex.tokens);
							free(tokens);
							clear_node(root);
							continue;
						}

						if (has_name == 0) {
							__int128 num = get_result(root);
							for (int i = 0; i < 8; i++)
								res.code[res.code_size - 8 + i] = (num >> (i << 3)) & 0xff;
						} else {
							char* exp = calloc(64,1);
							unsigned int exp_size = 0;

							for (int i = 0; i < count; i++) {
								unsigned int len = strlen(tokens[i].value);
								memcpy(&exp[exp_size], tokens[i].value, len);
								exp_size += len;
							}

							res.addrs = realloc(res.addrs, ++res.addrs_count * sizeof(Addr_sec_elem));
							Addr_sec_elem* addr = calloc(sizeof(Addr_sec_elem), 1);
							memcpy(addr->name, exp, exp_size);
							addr->offset = res.code_size - 8;
							memcpy(&res.addrs[res.addrs_count-1], addr, sizeof(Addr_sec_elem));
							free(addr);
							free(exp);
						}

						clear_node(root);
					}

					free(tokens);
				}
			}
		}

		res.ok &= expect_new_line(lex.tokens);
	}

	return res;
}
