#ifndef TREE_TEX_DUMP_H
#define TREE_TEX_DUMP_H

#include "tree.h"

Errors_of_tree create_latex_file(FILE *file_pointer);
Errors_of_tree close_latex_file(FILE *file_pointer);

struct Function_tex_view
{
    Function_name name;
    void (*print_function) (struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
};

void latex_dump(struct Node *root, FILE *file_pointer, const char *action);
void special_latex_dump(const char *str, FILE *file_pointer, const char *action);
const char * transform_variable_to_str(struct Value value);
const char transform_operation_to_str(struct Value value);

#endif
