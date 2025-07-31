#ifndef TREE_INPUT_H
#define TREE_INPUT_H

#include "tree.h"


Errors_of_tree get_tree_from_file(struct Tree *tree, const char *file_source_name);
void transform_to_arithmetic_operation(const char symbol, struct Value *value);
#endif
