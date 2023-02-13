#include <math_parser.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils.h>


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
			print_node(root,1);
			root->value->value[0] = c;
			putc('\n', stdout);*/

			if (cur->value->type == TYPE_ADDR_START || cur->value->type == TYPE_ADDR_END || cur->value->type == TYPE_GLOBAL_ADDR_START || cur->value->type == TYPE_GLOBAL_ADDR_END) {
				free(cur);
				continue;
			}

			if (cur->value->type == TYPE_NUMBER || cur->value->type == TYPE_HEX_NUMBER || cur->value->type == TYPE_UNDEFINED) {
				if (root->value->type == TYPE_NUMBER || root->value->type == TYPE_HEX_NUMBER || root->value->type == TYPE_UNDEFINED) {
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

__int128 get_result(Node* node) {
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
