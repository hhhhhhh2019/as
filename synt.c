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

	ti--;

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
	res.ok = 1;

	ti = 0;

	while (ti < lex.count) {
		Token t1 = lex.tokens[ti++];

		//	printf("%-3d %s\n", t1.type, t1.value);

		if (t1.type == TYPE_NEW_LINE)
			continue;

		if (match(t1, (char[]){TYPE_INSTRUCTION,TYPE_LABEL,TYPE_PREPROCESSOR}, 3) == 0) {
			next_line(lex.tokens);
			continue;
		}

		if (t1.type == TYPE_LABEL) {
			char* val = calloc(strlen(t1.value)+1,1);
			strcpy(val,t1.value);
			val[strlen(t1.value)-1] = 0;
			unsigned int id = -1;
			for (int i = 0; i < res.names_count; i++)
				if (strcmp(res.names[i].name, val) == 0) {
					id = i;
					break;
				}

			if (id != -1) {
				res.names[id].offset = res.code_size;
			} else {
				Name_sec_elem* name = calloc(sizeof(Name_sec_elem),1);
				strcpy(name->name, val);
				name->offset = res.code_size;

				res.names = realloc(res.names, ++res.names_count * sizeof(Name_sec_elem));
				memcpy(&res.names[res.names_count-1], name, sizeof(Name_sec_elem));
				free(name);
			}

			free(val);
		} else if (t1.type == TYPE_PREPROCESSOR) {
			if (strcmp(t1.value, ".byte\0") == 0) {
				Token t = lex.tokens[ti++];

				while (t.type != TYPE_NEW_LINE) {
					if (match(t, (char[]){TYPE_NUMBER,TYPE_HEX_NUMBER}, 2) == 1) {
						__int128 num = 0;

						if (t.type == TYPE_NUMBER)
							num = s2n(t.value);
						else
							num = h2n(t.value);

						res.code = realloc(res.code, ++res.code_size);
						res.code[res.code_size-1] = num & 0xff;
					}

					t = lex.tokens[ti++];
				}
			} else if (strcmp(t1.value, ".short\0") == 0) {
				Token t = lex.tokens[ti++];

				while (t.type != TYPE_NEW_LINE) {
					if (match(t, (char[]){TYPE_NUMBER,TYPE_HEX_NUMBER}, 2) == 1) {
						__int128 num = 0;

						if (t.type == TYPE_NUMBER)
							num = s2n(t.value);
						else
							num = h2n(t.value);

						res.code_size += 2;
						res.code = realloc(res.code, res.code_size);
						for (int i = 0; i < 2; i++)
							res.code[res.code_size-2+i] = (num << (i << 3)) & 0xff;
					}

					t = lex.tokens[ti++];
				}
			} else if (strcmp(t1.value, ".int\0") == 0) {
				Token t = lex.tokens[ti++];

				while (t.type != TYPE_NEW_LINE) {
					if (match(t, (char[]){TYPE_NUMBER,TYPE_HEX_NUMBER}, 2) == 1) {
						__int128 num = 0;

						if (t.type == TYPE_NUMBER)
							num = s2n(t.value);
						else
							num = h2n(t.value);

						res.code_size += 4;
						res.code = realloc(res.code, res.code_size);
						for (int i = 0; i < 4; i++)
							res.code[res.code_size-4+i] = (num << (i << 3)) & 0xff;
					}

					t = lex.tokens[ti++];
				}
			} else if (strcmp(t1.value, ".long\0") == 0) {
				Token t = lex.tokens[ti++];

				while (t.type != TYPE_NEW_LINE) {
					if (match(t, (char[]){TYPE_NUMBER,TYPE_HEX_NUMBER}, 2) == 1) {
						__int128 num = 0;

						if (t.type == TYPE_NUMBER)
							num = s2n(t.value);
						else
							num = h2n(t.value);

						res.code_size += 8;
						res.code = realloc(res.code, res.code_size);
						for (int i = 0; i < 8; i++)
							res.code[res.code_size-8+i] = (num << (i << 3)) & 0xff;
					}

					t = lex.tokens[ti++];
				}
			} else if (strcmp(t1.value, ".reg\0") == 0) {
				Token t = lex.tokens[ti++];

				while (t.type != TYPE_NEW_LINE) {
					if (match(t, (char[]){TYPE_NUMBER,TYPE_HEX_NUMBER}, 2) == 1) {
						__int128 num = 0;

						if (t.type == TYPE_NUMBER)
							num = s2n(t.value);
						else
							num = h2n(t.value);

						res.code_size += 16;
						res.code = realloc(res.code, res.code_size);
						for (int i = 0; i < 16; i++)
							res.code[res.code_size-16+i] = (num << (i << 3)) & 0xff;
					}

					t = lex.tokens[ti++];
				}
			}
		} else if (t1.type == TYPE_INSTRUCTION) {
			if (strcmp(t1.value, "nop\0") == 0) {
				res.code_size += 1;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size - 1] = 0x00; // nop
			} else if (strcmp(t1.value, "hlt\0") == 0) {
				res.code_size += 1;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size - 1] = 0x01; // hlt
			} else if (strcmp(t1.value, "movb\0") == 0) {
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
					ti++;

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
			} else if (strcmp(t1.value, "movs\0") == 0) {
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
						res.code[res.code_size - 3] = 0x08; // movs r
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
								res.code[res.code_size - 3] = 0x21; // movs from RAMr
								res.code[res.code_size - 2] = s2n(t2.value+1);
								res.code[res.code_size - 1] = s2n(tokens[1].value+1);
							}
						} else {
							res.code_size += 10;
							res.code = realloc(res.code, res.code_size);
							if (tokens[0].type == TYPE_ADDR_START)
								res.code[res.code_size -10] = 0x0d; // movs from RAM
							else
								res.code[res.code_size -10] = 0x17; // movs from RAMg
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
						res.code_size += 4;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 4] = 0x03; // movs n
						res.code[res.code_size - 3] = s2n(t2.value+1);
						res.code[res.code_size - 2] = 0;
						res.code[res.code_size - 1] = 0;

						if (has_name == 0) {
							Node* root = create_math_tree(tokens, count);
							unsigned int size = get_tree_size(root);
							if (size == count) {
								__int128 num = get_result(root);
								for (int i = 0; i < 2; i++)
									res.code[res.code_size - 2 + i] = (num >> (i << 3)) & 0xff;
							} else
								ti -= count - size + 1;
							clear_node(root);
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Использование ссылок вместе с \"movs\" не допускается!\n", tokens[0].line,tokens[0].offset);
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
					ti++;

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
							res.code[res.code_size - 3] = 0x26; // movs to RAMr
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
							res.code[res.code_size -10] = 0x12; // movs to RAM
						else
							res.code[res.code_size -10] = 0x1c; // movs to RAMg
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
			} else if (strcmp(t1.value, "movi\0") == 0) {
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
						res.code[res.code_size - 3] = 0x09; // movi r
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
								res.code[res.code_size - 3] = 0x22; // movi from RAMr
								res.code[res.code_size - 2] = s2n(t2.value+1);
								res.code[res.code_size - 1] = s2n(tokens[1].value+1);
							}
						} else {
							res.code_size += 10;
							res.code = realloc(res.code, res.code_size);
							if (tokens[0].type == TYPE_ADDR_START)
								res.code[res.code_size -10] = 0x0e; // movi from RAM
							else
								res.code[res.code_size -10] = 0x18; // movi from RAMg
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
						res.code_size += 6;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 6] = 0x04; // movi n
						res.code[res.code_size - 5] = s2n(t2.value+1);
						res.code[res.code_size - 4] = 0;
						res.code[res.code_size - 3] = 0;
						res.code[res.code_size - 2] = 0;
						res.code[res.code_size - 1] = 0;

						if (has_name == 0) {
							Node* root = create_math_tree(tokens, count);
							unsigned int size = get_tree_size(root);
							if (size == count) {
								__int128 num = get_result(root);
								for (int i = 0; i < 4; i++)
									res.code[res.code_size - 4 + i] = (num >> (i << 3)) & 0xff;
							} else
								ti -= count - size + 1;
							clear_node(root);
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Использование ссылок вместе с \"movi\" не допускается!\n", tokens[0].line,tokens[0].offset);
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
					ti++;

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
							res.code[res.code_size - 3] = 0x27; // movi to RAMr
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
							res.code[res.code_size -10] = 0x13; // movi to RAM
						else
							res.code[res.code_size -10] = 0x1d; // movi to RAMg
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
			} else if (strcmp(t1.value, "movl\0") == 0) {
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
						res.code[res.code_size - 3] = 0x0a; // movl r
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
								res.code[res.code_size - 3] = 0x23; // movl from RAMr
								res.code[res.code_size - 2] = s2n(t2.value+1);
								res.code[res.code_size - 1] = s2n(tokens[1].value+1);
							}
						} else {
							res.code_size += 10;
							res.code = realloc(res.code, res.code_size);
							if (tokens[0].type == TYPE_ADDR_START)
								res.code[res.code_size -10] = 0x0f; // movl from RAM
							else
								res.code[res.code_size -10] = 0x19; // movl from RAMg
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
						res.code_size += 10;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size -10] = 0x05; // movl n
						res.code[res.code_size - 9] = s2n(t2.value+1);
						res.code[res.code_size - 8] = 0;
						res.code[res.code_size - 7] = 0;
						res.code[res.code_size - 6] = 0;
						res.code[res.code_size - 5] = 0;
						res.code[res.code_size - 4] = 0;
						res.code[res.code_size - 3] = 0;
						res.code[res.code_size - 2] = 0;
						res.code[res.code_size - 1] = 0;

						if (has_name == 0) {
							Node* root = create_math_tree(tokens, count);
							unsigned int size = get_tree_size(root);
							if (size == count) {
								__int128 num = get_result(root);
								for (int i = 0; i < 8; i++)
									res.code[res.code_size - 8 + i] = (num >> (i << 3)) & 0xff;
							} else
								ti -= count - size + 1;
							clear_node(root);
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
					}

					free(tokens);
				} else if (t2.type == TYPE_ADDR_START || t2.type == TYPE_GLOBAL_ADDR_START) {
					unsigned int count = 0;
					char has_name = 0;
					char has_reg = 0;

					Token* tokens = get_exp(lex.tokens, t2.type+1, &count, &has_name);
					ti++;

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
							res.code[res.code_size - 3] = 0x28; // movl to RAMr
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
							res.code[res.code_size -10] = 0x14; // movl to RAM
						else
							res.code[res.code_size -10] = 0x1e; // movl to RAMg
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
			} else if (strcmp(t1.value, "movr\0") == 0) {
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
						res.code[res.code_size - 3] = 0x0b; // movr r
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
								res.code[res.code_size - 3] = 0x24; // movr from RAMr
								res.code[res.code_size - 2] = s2n(t2.value+1);
								res.code[res.code_size - 1] = s2n(tokens[1].value+1);
							}
						} else {
							res.code_size += 10;
							res.code = realloc(res.code, res.code_size);
							if (tokens[0].type == TYPE_ADDR_START)
								res.code[res.code_size -10] = 0x10; // movr from RAM
							else
								res.code[res.code_size -10] = 0x1a; // movr from RAMg
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
						res.code_size += 18;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size -18] = 0x06; // movr n
						res.code[res.code_size -17] = s2n(t2.value+1);
						res.code[res.code_size -16] = 0;
						res.code[res.code_size -15] = 0;
						res.code[res.code_size -14] = 0;
						res.code[res.code_size -13] = 0;
						res.code[res.code_size -12] = 0;
						res.code[res.code_size -11] = 0;
						res.code[res.code_size -10] = 0;
						res.code[res.code_size - 9] = 0;
						res.code[res.code_size - 8] = 0;
						res.code[res.code_size - 7] = 0;
						res.code[res.code_size - 6] = 0;
						res.code[res.code_size - 5] = 0;
						res.code[res.code_size - 4] = 0;
						res.code[res.code_size - 3] = 0;
						res.code[res.code_size - 2] = 0;
						res.code[res.code_size - 1] = 0;

						if (has_name == 0) {
							Node* root = create_math_tree(tokens, count);
							unsigned int size = get_tree_size(root);
							if (size == count) {
								__int128 num = get_result(root);
								for (int i = 0; i < 16; i++)
									res.code[res.code_size - 16 + i] = (num >> (i << 3)) & 0xff;
							} else
								ti -= count - size + 1;
							clear_node(root);
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
							addr->offset = res.code_size - 16;
							memcpy(&res.addrs[res.addrs_count-1], addr, sizeof(Addr_sec_elem));
							free(addr);
							free(exp);
						}
					}

					free(tokens);
				} else if (t2.type == TYPE_ADDR_START || t2.type == TYPE_GLOBAL_ADDR_START) {
					unsigned int count = 0;
					char has_name = 0;
					char has_reg = 0;

					Token* tokens = get_exp(lex.tokens, t2.type+1, &count, &has_name);
					ti++;

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
							res.code[res.code_size - 3] = 0x29; // movr to RAMr
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
							res.code[res.code_size -10] = 0x15; // movr to RAM
						else
							res.code[res.code_size -10] = 0x1f; // movr to RAMg
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
			} else if (strcmp(t1.value, "pushb\0") == 0) {
				unsigned int count = 0;
				char has_name = 0;
				Token* tokens = get_exp(lex.tokens, TYPE_NEW_LINE, &count, &has_name);

				if (tokens[0].type == TYPE_REGISTER) {
					res.code_size += 2;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size-2] = 0x2f; // pushb
					res.code[res.code_size-1] = 0;

					if (count == 1) {
						res.code[res.code_size-1] = s2n(tokens[0].value+1) & 0xff;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[1].line,tokens[1].offset);
						for (int i = 1; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						continue;
					}
				} else {
					Node* root = create_math_tree(tokens, count);

					unsigned int size = get_tree_size(root);

					if (size != count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[size].line,tokens[size].offset);
						for (int i = size; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						clear_node(root);
						continue;
					} else {
						res.code_size += 2;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size-2] = 0x2a; // pushb n
						res.code[res.code_size-1] = 0;

						if (has_name == 0) {
							__int128 num = get_result(root);
							res.code[res.code_size-1] = num & 0xff;
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Использование ссылок вместе с \"pushb\" не допускается!\n", tokens[0].line,tokens[0].offset);
							res.ok = 0;
							next_line(lex.tokens);
							free(tokens);
							clear_node(root);
							continue;
						}
					}

					clear_node(root);
				}

				free(tokens);
			} else if (strcmp(t1.value, "pushs\0") == 0) {
				unsigned int count = 0;
				char has_name = 0;
				Token* tokens = get_exp(lex.tokens, TYPE_NEW_LINE, &count, &has_name);

				if (tokens[0].type == TYPE_REGISTER) {
					res.code_size += 2;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size-2] = 0x30; // pushs
					res.code[res.code_size-1] = 0;

					if (count == 1) {
						res.code[res.code_size-1] = s2n(tokens[0].value+1) & 0xff;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[1].line,tokens[1].offset);
						for (int i = 1; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						continue;
					}
				} else {
					Node* root = create_math_tree(tokens, count);

					unsigned int size = get_tree_size(root);

					if (size != count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[size].line,tokens[size].offset);
						for (int i = size; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						clear_node(root);
						continue;
					} else {
						res.code_size += 3;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size-3] = 0x2b; // pushs n
						res.code[res.code_size-2] = 0;
						res.code[res.code_size-1] = 0;

						if (has_name == 0) {
							__int128 num = get_result(root);
							for (int i = 0; i < 2; i++)
								res.code[res.code_size-2+i] = (num >> (i << 3)) & 0xff;
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Использование ссылок вместе с \"pushs\" не допускается!\n", tokens[0].line,tokens[0].offset);
							res.ok = 0;
							next_line(lex.tokens);
							free(tokens);
							clear_node(root);
							continue;
						}
					}

					clear_node(root);
				}

				free(tokens);
			} else if (strcmp(t1.value, "pushi\0") == 0) {
				unsigned int count = 0;
				char has_name = 0;
				Token* tokens = get_exp(lex.tokens, TYPE_NEW_LINE, &count, &has_name);

				if (tokens[0].type == TYPE_REGISTER) {
					res.code_size += 2;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size-2] = 0x31; // pushi
					res.code[res.code_size-1] = 0;

					if (count == 1) {
						res.code[res.code_size-1] = s2n(tokens[0].value+1) & 0xff;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[1].line,tokens[1].offset);
						for (int i = 1; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						continue;
					}
				} else {
					Node* root = create_math_tree(tokens, count);

					unsigned int size = get_tree_size(root);

					if (size != count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[size].line,tokens[size].offset);
						for (int i = size; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						clear_node(root);
						continue;
					} else {
						res.code_size += 5;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size-5] = 0x2c; // pushi n
						res.code[res.code_size-4] = 0;
						res.code[res.code_size-3] = 0;
						res.code[res.code_size-2] = 0;
						res.code[res.code_size-1] = 0;

						if (has_name == 0) {
							__int128 num = get_result(root);
							for (int i = 0; i < 4; i++)
								res.code[res.code_size-4+i] = (num >> (i << 3)) & 0xff;
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: Использование ссылок вместе с \"pushi\" не допускается!\n", tokens[0].line,tokens[0].offset);
							res.ok = 0;
							next_line(lex.tokens);
							free(tokens);
							clear_node(root);
							continue;
						}
					}

					clear_node(root);
				}

				free(tokens);
			} else if (strcmp(t1.value, "pushl\0") == 0) {
				unsigned int count = 0;
				char has_name = 0;
				Token* tokens = get_exp(lex.tokens, TYPE_NEW_LINE, &count, &has_name);

				if (tokens[0].type == TYPE_REGISTER) {
					res.code_size += 2;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size-2] = 0x32; // pushl
					res.code[res.code_size-1] = 0;

					if (count == 1) {
						res.code[res.code_size-1] = s2n(tokens[0].value+1) & 0xff;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[1].line,tokens[1].offset);
						for (int i = 1; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						continue;
					}
				} else {
					Node* root = create_math_tree(tokens, count);

					unsigned int size = get_tree_size(root);

					if (size != count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[size].line,tokens[size].offset);
						for (int i = size; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						clear_node(root);
						continue;
					} else {
						res.code_size += 9;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size-9] = 0x2d; // pushl n
						res.code[res.code_size-8] = 0;
						res.code[res.code_size-7] = 0;
						res.code[res.code_size-6] = 0;
						res.code[res.code_size-5] = 0;
						res.code[res.code_size-4] = 0;
						res.code[res.code_size-3] = 0;
						res.code[res.code_size-2] = 0;
						res.code[res.code_size-1] = 0;

						if (has_name == 0) {
							__int128 num = get_result(root);
							for (int i = 0; i < 9; i++)
								res.code[res.code_size-9+i] = (num >> (i << 3)) & 0xff;
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
					}

					clear_node(root);
				}

				free(tokens);
			} else if (strcmp(t1.value, "pushr\0") == 0) {
				unsigned int count = 0;
				char has_name = 0;
				Token* tokens = get_exp(lex.tokens, TYPE_NEW_LINE, &count, &has_name);

				if (tokens[0].type == TYPE_REGISTER) {
					res.code_size += 2;
					res.code = realloc(res.code, res.code_size);
					res.code[res.code_size-2] = 0x33; // pushr
					res.code[res.code_size-1] = 0;

					if (count == 1) {
						res.code[res.code_size-1] = s2n(tokens[0].value+1) & 0xff;
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[1].line,tokens[1].offset);
						for (int i = 1; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						continue;
					}
				} else {
					Node* root = create_math_tree(tokens, count);

					unsigned int size = get_tree_size(root);

					if (size != count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Найден мусор после выражения!: ", tokens[size].line,tokens[size].offset);
						for (int i = size; i < count; i++)
							printf("%s ", tokens[i].value);
						putc('\n', stdout);
						res.ok = 0;
						next_line(lex.tokens);
						free(tokens);
						clear_node(root);
						continue;
					} else {
						res.code_size += 17;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size-17] = 0x2e; // pushr n
						res.code[res.code_size-16] = 0;
						res.code[res.code_size-15] = 0;
						res.code[res.code_size-14] = 0;
						res.code[res.code_size-13] = 0;
						res.code[res.code_size-12] = 0;
						res.code[res.code_size-11] = 0;
						res.code[res.code_size-10] = 0;
						res.code[res.code_size- 9] = 0;
						res.code[res.code_size- 8] = 0;
						res.code[res.code_size- 7] = 0;
						res.code[res.code_size- 6] = 0;
						res.code[res.code_size- 5] = 0;
						res.code[res.code_size- 4] = 0;
						res.code[res.code_size- 3] = 0;
						res.code[res.code_size- 2] = 0;
						res.code[res.code_size- 1] = 0;

						if (has_name == 0) {
							__int128 num = get_result(root);
							for (int i = 0; i < 16; i++)
								res.code[res.code_size-16+i] = (num >> (i << 3)) & 0xff;
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
							addr->offset = res.code_size - 16;
							memcpy(&res.addrs[res.addrs_count-1], addr, sizeof(Addr_sec_elem));
							free(addr);
							free(exp);
						}
					}

					clear_node(root);
				}

				free(tokens);
			} else if (strcmp(t1.value, "popb\0\0") == 0) {
				Token t2 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 2;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-2] = 0x34; // popb
				res.code[res.code_size-1] = s2n(t2.value+1) & 0xff;
			} else if (strcmp(t1.value, "pops\0\0") == 0) {
				Token t2 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 2;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-2] = 0x35; // pops
				res.code[res.code_size-1] = s2n(t2.value+1) & 0xff;
			} else if (strcmp(t1.value, "popi\0\0") == 0) {
				Token t2 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 2;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-2] = 0x36; // popi
				res.code[res.code_size-1] = s2n(t2.value+1) & 0xff;
			} else if (strcmp(t1.value, "popl\0\0") == 0) {
				Token t2 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 2;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-2] = 0x37; // popl
				res.code[res.code_size-1] = s2n(t2.value+1) & 0xff;
			} else if (strcmp(t1.value, "popr\0\0") == 0) {
				Token t2 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 2;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-2] = 0x38; // popr
				res.code[res.code_size-1] = s2n(t2.value+1) & 0xff;
			} else if (strcmp(t1.value, "sumb\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x39; // sumb
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "sums\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x3a; // sums
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "sumi\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x3b; // sumi
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "suml\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x3c; // suml
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "sumr\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x3d; // sumr
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "subb\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x3e; // subb
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "subs\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x3f; // subs
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "subi\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x40; // subi
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "subl\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x41; // subl
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "subr\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x42; // subr
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "mulb\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x43; // mulb
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "muls\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x44; // muls
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "muli\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x45; // muli
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "mull\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x46; // mull
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "mulr\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x47; // mulr
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "imulb\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x48; // imulb
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "imuls\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x49; // imuls
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "imuli\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x4a; // imuli
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "imull\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x4b; // imull
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "imulr\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x4c; // imulr
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "divb\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x4d; // divb
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "divs\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x4e; // divs
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "divi\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x4f; // divi
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "divl\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x50; // divl
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "divr\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x51; // divr
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "idivb\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x52; // idivb
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "idivs\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-3] = 0x53; // idivs
				res.code[res.code_size-2] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "idivi\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x54; // idivi
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "idivl\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x55; // idivl
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "idivr\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];
				Token t4 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t4, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 4;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-4] = 0x56; // idivr
				res.code[res.code_size-3] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-2] = s2n(t3.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t4.value+1) & 0xff;
			} else if (strcmp(t1.value, "cmpb\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 3;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-3] = 0x57; // cmpb
				res.code[res.code_size-2] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t3.value+1) & 0xff;
			} else if (strcmp(t1.value, "cmps\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 3;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-3] = 0x58; // cmps
				res.code[res.code_size-2] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t3.value+1) & 0xff;
			} else if (strcmp(t1.value, "cmpi\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 3;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-3] = 0x59; // cmpi
				res.code[res.code_size-2] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t3.value+1) & 0xff;
			} else if (strcmp(t1.value, "cmpl\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 3;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-3] = 0x5a; // cmpl
				res.code[res.code_size-2] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t3.value+1) & 0xff;
			} else if (strcmp(t1.value, "cmpr\0\0") == 0) {
				Token t2 = lex.tokens[ti++];
				Token t3 = lex.tokens[ti++];

				if (match(t2, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				if (match(t3, (char[]){TYPE_REGISTER}, 1) == 0) {
					next_line(lex.tokens);
					res.ok = 0;
					continue;
				}

				res.code_size += 3;
				res.code = realloc(res.code, res.code_size);
				res.code[res.code_size-3] = 0x5b; // cmpr
				res.code[res.code_size-2] = s2n(t2.value+1) & 0xff;
				res.code[res.code_size-1] = s2n(t3.value+1) & 0xff;
			}
		}

		res.ok &= expect_new_line(lex.tokens);
	}

	return res;
}
