#ifndef MATH_P_H
#define MATH_P_H

#include <token.h>

typedef struct {
	void* parent;
	void* left;
	void* right;
	Token* value;
} Node;


Node* create_math_tree(Token*, unsigned int);
void print_node(Node*, int);
void clear_node(Node*);
__int128 get_result(Node*);
int get_tree_size(Node*);


#endif // MATH_P_H
