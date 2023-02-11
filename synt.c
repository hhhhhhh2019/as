#include <synt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>


typedef struct {
	void* parent;
	void* left;
	void* right;
	Token* value;
} Node;


void print_node(Node*, int);


Node* create_math_tree(Token* t, unsigned int count) {
	if (count == 0)
		return 0;

	Node* root = NULL;

	for (int i = 0; i < count; i++) {
		Node* cur = calloc(sizeof(Node),1);
		cur->value = &t[i];

		if (root == NULL) {
			root = cur;
		} else {
			/*printf("%s %s\n", root->value->value, cur->value->value);
			char c = root->value->value[0];
			root->value->value[0] = '.';
			print_tree(root,1);
			root->value->value[0] = c;
			putc('\n', stdout);*/

			if (cur->value->type == TYPE_NUMBER || cur->value->type == TYPE_HEX_NUMBER) {
				if (root->value->type == TYPE_NUMBER || root->value->type == TYPE_HEX_NUMBER) {
					free(cur);
					return root;
				}

				if (root->left == NULL) {
					root->left = cur;
					cur->parent = root;
				} else if (root->right == NULL) {
					root->right = cur;
					cur->parent = root;
				} else {
					free(cur);
					break;
				}

				if (root->value->type == TYPE_BRACKET_START) {
					root = cur;
				}
			}

			if (cur->value->type == TYPE_SUM || cur->value->type == TYPE_SUB) {
				if (root->parent != NULL)
					((Node*)root->parent)->left = cur;
				cur->parent = root->parent;
				cur->left = root;
				root->parent = cur;
				root = cur;
			}

			if (cur->value->type == TYPE_MUL || cur->value->type == TYPE_DIV) {
				if (root->value->type == TYPE_NUMBER || root->value->type == TYPE_MUL || root->value->type == TYPE_BRACKET_START) {
					if (root->parent != NULL)
						((Node*)root->parent)->left = cur;
					cur->parent = root->parent;
					cur->left = root;
					root->parent = cur;
					root = cur;
				}

				if (root->value->type == TYPE_SUM || root->value->type == TYPE_SUB) {
					cur->parent = root;
					cur->left = root->right;
					((Node*)cur->left)->parent = cur;
					root->right = cur;
					root = cur;
				}
			}

			if (cur->value->type == TYPE_BRACKET_START) {
				if (root->left == NULL) {
					root->left = cur;
					cur->parent = root;
				} else if (root->right == NULL) {
					root->right = cur;
					cur->parent = root;
				} else {
					free(cur);
					break;
				}

				root = cur;
			}

			if (cur->value->type == TYPE_BRACKET_END) {
				while (root->value->type != TYPE_BRACKET_START)
					root = root->parent;

				root->right = cur;
				cur->parent = root;

				if (root->parent != NULL && ((Node*)root->parent)->value->type != TYPE_BRACKET_START)
					root = root->parent;
			}
		}
	}

	while (root->parent != NULL)
		root = root->parent;

	return root;
}

void clear_node(Node* node) {
	if (node == NULL)
		return;

	clear_node(node->left);
	clear_node(node->right);

	free(node);
}

void print_node(Node* node, int l) {
	if (l == 1)
		while (node->parent != NULL)
			node = node->parent;

	if (node == NULL)
		return;

	char* format = calloc(100,1);
	sprintf(format, "%%%ds\n", l);
	printf(format, node->value->value);
	print_node(node->left, l+2);
	print_node(node->right, l+2);
	free(format);
}

int get_result(Node* node) {
	if (node->value->type == TYPE_BRACKET_START) {
		return get_result(node->left);
	}
	else if (node->value->type == TYPE_NUMBER) {
		return s2n(node->value->value);
	} else if (node->value->type == TYPE_HEX_NUMBER) {
		return h2n(node->value->value);
	} else {
		if (node->value->type == TYPE_SUM) {
			return get_result(node->left) + get_result(node->right);
		} else if (node->value->type == TYPE_SUB) {
			return get_result(node->left) - get_result(node->right);
		} else if (node->value->type == TYPE_MUL) {
			return get_result(node->left) * get_result(node->right);
		} else if (node->value->type == TYPE_DIV) {
			return get_result(node->left) / get_result(node->right);
		}
	}
}

int get_tree_size(Node* node) {
	if (node == NULL)
		return 0;

	return 1 + get_tree_size(node->left) + get_tree_size(node->right);
}


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

		if (t1.type == TYPE_NEW_LINE) {}
		else if (t1.type == TYPE_LABEL) {

		} else if (t1.type == TYPE_INSTRUCTION) {
			if (strcmp(t1.value, "nop\0") == 0) {
				res.code = realloc(res.code, ++res.code_size);
				res.code[res.code_size - 1] = 0x00;
			} else if (strcmp(t1.value, "hlt\0") == 0) {
				res.code = realloc(res.code, ++res.code_size);
				res.code[res.code_size - 1] = 0x01;
			} else if (strcmp(t1.value, "movb\0") == 0) {
				Token t2 = lex.tokens[i];

				if (t2.type == TYPE_REGISTER) {
					Token t = lex.tokens[++i];

					if (t.type == TYPE_ADDR_START) {
						Token *tokens = malloc(0);
						unsigned int tokens_count = 0;
						char has_name = 0;
						unsigned int end_id = 0;

						while (t.type != TYPE_NEW_LINE) {
							if (t.type == TYPE_UNDEFINED)
								has_name = 1;

							if (t.type == TYPE_ADDR_END)
								end_id = i - 1;

							tokens = realloc(tokens, (++tokens_count) * sizeof(Token));

							memcpy(&tokens[tokens_count-1], &t, sizeof(Token));

							t = lex.tokens[++i];
						}

						if (end_id != tokens_count) {
							printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[end_id].line,tokens[end_id].offset);
							for (int i = end_id; i < tokens_count; i++)
								printf("%s ", tokens[i].value);
							putc('\n', stdout);
							res.ok = 0;
						} else {
							if (tokens_count == 3 && tokens[1].type == TYPE_REGISTER) {
								res.code_size += 2 + 1;
								res.code = realloc(res.code, res.code_size);
								res.code[res.code_size - 3] = 0x20; // movb from RAMr
								res.code[res.code_size - 2] = s2n(t2.value+1) & 0xff;
								res.code[res.code_size - 1] = s2n(tokens[1].value+1) & 0xff;
							} else {
								res.code_size += 2 + 8;
								res.code = realloc(res.code, res.code_size);
								res.code[res.code_size - 10] = 0x0c; // movb from RAM
								res.code[res.code_size - 9] = s2n(t2.value+1) & 0xff;
								res.code[res.code_size - 8] = 0;
								res.code[res.code_size - 7] = 0;
								res.code[res.code_size - 6] = 0;
								res.code[res.code_size - 5] = 0;
								res.code[res.code_size - 4] = 0;
								res.code[res.code_size - 3] = 0;
								res.code[res.code_size - 2] = 0;
								res.code[res.code_size - 1] = 0;

								if (has_name == 1) {
									Node* root = create_math_tree(tokens, tokens_count);

									while (root->parent != NULL)
										root = root->parent;

									int root_size = get_tree_size(root) + 2; // [, ]

									if (root_size == tokens_count) {
										char* exp = calloc(64,1);
										unsigned char exp_size = 0;

										for (int j = 1; j < tokens_count - 1; j++) {
											memcpy(&exp[exp_size], tokens[j].value, strlen(tokens[j].value));
											exp_size += strlen(tokens[j].value);
										}

										res.addrs = realloc(res.addrs, sizeof(Addr_sec_elem) * (++res.addrs_count));

										Addr_sec_elem addr;
										memcpy(addr.name, exp, 64);
										addr.offset = res.code_size - 8;

										memcpy(&res.addrs[res.addrs_count - 1], &addr, sizeof(Addr_sec_elem));

										free(exp);
									} else {
										printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size-1].line,tokens[root_size-1].offset);
										for (int i = root_size - 1; i < tokens_count - 1; i++)
											printf("%s ", tokens[i].value);
										putc('\n', stdout);
										res.ok = 0;
									}

									clear_node(root);
								} else {
									Node* root = create_math_tree(tokens, tokens_count);

									while (root->parent != NULL)
										root = root->parent;

									int root_size = get_tree_size(root) + 2; // [, ]

									if (root_size == tokens_count) {
										__int128 num = get_result(root);

										for (int j = 0; j < 8; j++) {
											res.code[res.code_size - 8 + j] = (num >> (j << 3)) & 0xff;
										}
									} else {
										printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size-1].line,tokens[root_size-1].offset);
										for (int i = root_size - 1; i < tokens_count - 1; i++)
											printf("%s ", tokens[i].value);
										putc('\n', stdout);
										res.ok = 0;
									}

									clear_node(root);
								}
							}
						}

						free(tokens);
					} else if (t.type == TYPE_GLOBAL_ADDR_START) {
						Token *tokens = malloc(0);
						unsigned int tokens_count = 0;
						char has_name = 0;
						unsigned int end_id = 0;

						while (t.type != TYPE_NEW_LINE) {
							if (t.type == TYPE_UNDEFINED)
								has_name = 1;

							if (t.type == TYPE_GLOBAL_ADDR_END)
								end_id = i - 1;

							tokens = realloc(tokens, (++tokens_count) * sizeof(Token));

							memcpy(&tokens[tokens_count-1], &t, sizeof(Token));

							t = lex.tokens[++i];
						}

						if (end_id != tokens_count) {
							printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[end_id].line,tokens[end_id].offset);
							for (int i = end_id; i < tokens_count; i++)
								printf("%s ", tokens[i].value);
							putc('\n', stdout);
							res.ok = 0;
						} else {
							res.code_size += 2 + 8;
							res.code = realloc(res.code, res.code_size);
							res.code[res.code_size - 10] = 0x16; // movb from RAMg
							res.code[res.code_size - 9] = s2n(t2.value+1) & 0xff;
							res.code[res.code_size - 8] = 0;
							res.code[res.code_size - 7] = 0;
							res.code[res.code_size - 6] = 0;
							res.code[res.code_size - 5] = 0;
							res.code[res.code_size - 4] = 0;
							res.code[res.code_size - 3] = 0;
							res.code[res.code_size - 2] = 0;
							res.code[res.code_size - 1] = 0;

							if (has_name == 1) {
								Node* root = create_math_tree(tokens, tokens_count);

								int root_size = get_tree_size(root) + 2; // {, }

								if (root_size == tokens_count) {
									char* exp = calloc(64,1);
									unsigned char exp_size = 0;

									for (int j = 1; j < tokens_count - 1; j++) {
										memcpy(&exp[exp_size], tokens[j].value, strlen(tokens[j].value));
										exp_size += strlen(tokens[j].value);
									}

									res.addrs = realloc(res.addrs, sizeof(Addr_sec_elem) * (++res.addrs_count));

									Addr_sec_elem addr;
									memcpy(addr.name, exp, 64);
									addr.offset = res.code_size - 8;

									memcpy(&res.addrs[res.addrs_count - 1], &addr, sizeof(Addr_sec_elem));

									free(exp);
								} else {
									printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size-1].line,tokens[root_size-1].offset);
									for (int i = root_size - 1; i < tokens_count - 1; i++)
										printf("%s ", tokens[i].value);
									putc('\n', stdout);
									res.ok = 0;
								}

								clear_node(root);
							} else {
								Node* root = create_math_tree(tokens, tokens_count);

								int root_size = get_tree_size(root) + 2; // [, ]

								if (root_size == tokens_count) {
									__int128 num = get_result(root);

									for (int j = 0; j < 8; j++) {
										res.code[res.code_size - 8 + j] = (num >> (j << 3)) & 0xff;
									}
								} else {
									printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size-1].line,tokens[root_size-1].offset);
									for (int i = root_size - 1; i < tokens_count - 1; i++)
										printf("%s ", tokens[i].value);
									putc('\n', stdout);
									res.ok = 0;
								}

								clear_node(root);
							}
						}

						free(tokens);
					} else if (t.type != TYPE_NEW_LINE) {
						Token *tokens = malloc(0);
						unsigned int tokens_count = 0;
						char has_name = 0;

						while (t.type != TYPE_NEW_LINE) {
							if (t.type == TYPE_UNDEFINED)
								has_name = 1;

							tokens = realloc(tokens, (++tokens_count) * sizeof(Token));

							memcpy(&tokens[tokens_count-1], &t, sizeof(Token));

							t = lex.tokens[++i];
						}

						res.code_size += 2 + 1;
						res.code = realloc(res.code, res.code_size);
						res.code[res.code_size - 3] = 0x02; // movb n
						res.code[res.code_size - 2] = s2n(t2.value+1) & 0xff;
						res.code[res.code_size - 1] = 0;

						if (has_name == 1) {
							printf("\e[1;31mОшибка!\e[m %d,%d: Размер инструкции не соотвествует размеру адреса!\n", tokens[0].line,tokens[0].offset);
						} else {
							Node* root = create_math_tree(tokens, tokens_count);

							int root_size = get_tree_size(root);

							print_node(root, 1);

							if (root_size == tokens_count) {
								__int128 num = get_result(root);

								res.code[res.code_size - 1] = num & 0xff;
							} else {
								printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size].line,tokens[root_size].offset);
								for (int j = root_size; j < tokens_count; j++)
									printf("%s ", tokens[j].value);
								putc('\n', stdout);
								res.ok = 0;
							}

							clear_node(root);
						}

						free(tokens);
					} else {
						printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\" Ожидался регистр/число/адрес!\n", t.line,t.offset,t.value);
					}
				} else if (t2.type == TYPE_ADDR_START) {
					Token t = t2;

					Token *tokens = malloc(0);
					unsigned int tokens_count = 0;
					char has_name = 0;
					unsigned int end_id = 0;

					while (t.type != TYPE_NEW_LINE && t.type != TYPE_REGISTER) {
						if (t.type == TYPE_UNDEFINED)
							has_name = 1;

						if (t.type == TYPE_ADDR_END)
							end_id = i;

						tokens = realloc(tokens, (++tokens_count) * sizeof(Token));

						memcpy(&tokens[tokens_count-1], &t, sizeof(Token));

						t = lex.tokens[++i];
					}

					if (end_id == 0) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Конец выражения не обнаружен!\n", t2.line,t2.offset);
						res.ok = 0;
					} else if (end_id != tokens_count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[end_id].line,tokens[end_id].offset);
						for (int j = end_id; j < tokens_count; j++)
							printf("%s ", tokens[j].value);
						putc('\n', stdout);
						res.ok = 0;
					} else if (tokens_count == 3 && tokens[1].type == TYPE_REGISTER) {
						Token t3 = lex.tokens[i++];

						if (t3.type == TYPE_REGISTER) {
							res.code_size = 2 + 1;
							res.code = realloc(res.code, res.code_size);
							res.code[res.code_size - 3] = 0x25; // movb to RAMr
							res.code[res.code_size - 2] = s2n(t3.value+1) & 0xff;
							res.code[res.code_size - 1] = s2n(tokens[1].value+1) & 0xff;
						} else {
							printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидался регистр!\n", t3.line,t3.offset, t3.value);
							res.ok = 0;
						}
					} else {
						Node* root = create_math_tree(tokens, tokens_count);
						unsigned int root_size = get_tree_size(root);

						if (root_size != tokens_count - 2) {
							printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size].line,tokens[root_size].offset);
							for (int j = root_size + 1; j < tokens_count - 1; j++)
								printf("%s ", tokens[j].value);
							putc('\n', stdout);
							res.ok = 0;
						} else {
							Token t3 = lex.tokens[i++];

							if (t3.type == TYPE_REGISTER) {
								res.code_size = 2 + 8;
								res.code = realloc(res.code, res.code_size);
								res.code[res.code_size -10] = 0x11; // movb to RAM
								res.code[res.code_size - 9] = s2n(t3.value+1) & 0xff;
								res.code[res.code_size - 8] = 0;
								res.code[res.code_size - 7] = 0;
								res.code[res.code_size - 6] = 0;
								res.code[res.code_size - 5] = 0;
								res.code[res.code_size - 4] = 0;
								res.code[res.code_size - 3] = 0;
								res.code[res.code_size - 2] = 0;
								res.code[res.code_size - 1] = 0;

								if (has_name == 1) {
									char* exp = calloc(64,1);
									unsigned char exp_size = 0;

									for (int j = 1; j < tokens_count - 1; j++) {
										memcpy(&exp[exp_size], tokens[j].value, strlen(tokens[j].value));
										exp_size += strlen(tokens[j].value);
									}

									res.addrs = realloc(res.addrs, sizeof(Addr_sec_elem) * (++res.addrs_count));

									Addr_sec_elem addr;
									memcpy(addr.name, exp, 64);
									addr.offset = res.code_size - 8;

									memcpy(&res.addrs[res.addrs_count - 1], &addr, sizeof(Addr_sec_elem));

									free(exp);
								} else {
									__int128 num = get_result(root);

									for (int j = 0; j < 8; j++)
										res.code[res.code_size - 8 + j] = (num >> (j << 3)) & 0xff;
								}
							} else {
								printf("\e[1;31mОшибка!\e[m %d,%d: \"%s\": Ожидался регистр!\n", t3.line,t3.offset, t3.value);
								res.ok = 0;
							}
						}

						clear_node(root);
					}

					free(tokens);
				} else if (t2.type == TYPE_GLOBAL_ADDR_START) {
					Token t = t2;

					Token *tokens = malloc(0);
					unsigned int tokens_count = 0;
					char has_name = 0;
					unsigned int end_id = 0;

					while (t.type != TYPE_NEW_LINE && t.type != TYPE_REGISTER) {
						if (t.type == TYPE_UNDEFINED)
							has_name = 1;

						if (t.type == TYPE_GLOBAL_ADDR_END)
							end_id = i;

						tokens = realloc(tokens, (++tokens_count) * sizeof(Token));

						memcpy(&tokens[tokens_count-1], &t, sizeof(Token));

						t = lex.tokens[++i];
					}

					if (end_id == 0) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Конец выражения не обнаружен!\n", t2.line,t2.offset);
						res.ok = 0;
					} else if (end_id != tokens_count) {
						printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[end_id].line,tokens[end_id].offset);
						for (int j = end_id; j < tokens_count; j++)
							printf("%s ", tokens[j].value);
						putc('\n', stdout);
						res.ok = 0;
					} else {
						Node* root = create_math_tree(tokens, tokens_count);
						unsigned int root_size = get_tree_size(root);

						if (root_size != tokens_count - 2) {
							printf("\e[1;31mОшибка!\e[m %d,%d: Обнаружен мусор после выражения! ", tokens[root_size].line,tokens[root_size].offset);
							for (int j = root_size + 1; j < tokens_count - 1; j++)
								printf("%s ", tokens[j].value);
							putc('\n', stdout);
							res.ok = 0;
						} else {
							Token t3 = lex.tokens[i++];

							if (t3.type == TYPE_REGISTER) {
								res.code_size = 2 + 8;
								res.code = realloc(res.code, res.code_size);
								res.code[res.code_size -10] = 0x1b; // movb to RAMg
								res.code[res.code_size - 9] = s2n(t3.value+1) & 0xff;
								res.code[res.code_size - 8] = 0;
								res.code[res.code_size - 7] = 0;
								res.code[res.code_size - 6] = 0;
								res.code[res.code_size - 5] = 0;
								res.code[res.code_size - 4] = 0;
								res.code[res.code_size - 3] = 0;
								res.code[res.code_size - 2] = 0;
								res.code[res.code_size - 1] = 0;

								if (has_name == 1) {
									char* exp = calloc(64,1);
									unsigned char exp_size = 0;

									for (int j = 1; j < tokens_count - 1; j++) {
										memcpy(&exp[exp_size], tokens[j].value, strlen(tokens[j].value));
										exp_size += strlen(tokens[j].value);
									}

									res.addrs = realloc(res.addrs, sizeof(Addr_sec_elem) * (++res.addrs_count));

									Addr_sec_elem addr;
									memcpy(addr.name, exp, 64);
									addr.offset = res.code_size - 8;

									memcpy(&res.addrs[res.addrs_count - 1], &addr, sizeof(Addr_sec_elem));

									free(exp);
								} else {
									__int128 num = get_result(root);

									for (int j = 0; j < 8; j++)
										res.code[res.code_size - 8 + j] = (num >> (j << 3)) & 0xff;
								}
							}
						}

						clear_node(root);
					}

					free(tokens);
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
