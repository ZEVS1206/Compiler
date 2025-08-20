#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_tex_dump.h"

static void latex_bypass_of_tree(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void analyze_root(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void print_fraction(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void print_mul(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void print_add_or_sub(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void print_pow(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void print_one_node(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node);
static void print_function_sin(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);
static void print_function_cos(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call);

const struct Function_tex_view latex_view_functions[] =
{
    {FUNC_SIN, print_function_sin},
    {FUNC_COS, print_function_cos}
};

const size_t size_of_latex_view_functions = sizeof(latex_view_functions) / sizeof(latex_view_functions[0]);


const char * transform_variable_to_str(struct Value value)
{
    switch(value.variable)
    {
        case VAR:     return "x";    break;
        case NOT_A_VAR: return "NOT A VAR"; break;
    }
    return "NOT A VAR";
}

const char transform_operation_to_str(struct Value value)
{
    switch (value.operation)
    {
        case OP_ADD:    return '+';  break;
        case OP_SUB:    return '-';  break;
        case OP_MUL:    return '*';  break;
        case OP_DIV:    return '/';  break;
        case OP_DEG:    return '^';  break;
        case NOT_AN_OP: return '\n'; break;
        default:        return '\0'; break;
    }
    return '\n';
}


void special_latex_dump(const char *str, FILE *file_pointer, const char *action)
{
    if (file_pointer == NULL)
    {
        return;
    }
    fprintf(file_pointer, "%s:  \\texbf{\\large %s \\normalsize}\n", action, str);
    return;
}

static void print_function_cos(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    //% (in print_cos\n");
    if ((root->value).type == FUNCTION && (root == operation_for_print || root->parent_node == operation_for_print) && root == address_of_call)
    {
        fprintf(file_pointer, "\\begin{equation}\n");
        fprintf(file_pointer, "cos (");
        // if (((root->left)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s", transform_variable_to_str((root->left)->value));
        // }
        *is_it_possible_to_print_a_single_node = true;
        latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->left);
        *is_it_possible_to_print_an_operation = false;
        fprintf(file_pointer, ")\n");
        fprintf(file_pointer, "\n \\end{equation}\n");
    }
    //% (in print_cos\n");
    return;
}


static void print_function_sin(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    //% (in print_sin\n");
    if ((root->value).type == FUNCTION && (root == operation_for_print || root->parent_node == operation_for_print) && root == address_of_call)
    {
        fprintf(file_pointer, "\\begin{equation}\n");
        fprintf(file_pointer, "sin (");
        // if (((root->left)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s", transform_variable_to_str((root->left)->value));
        // }
        *is_it_possible_to_print_a_single_node = true;
        latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->left);
        *is_it_possible_to_print_an_operation = false;
        fprintf(file_pointer, ")\n");
        fprintf(file_pointer, "\n \\end{equation}\n");
    }//% (in print_sin\n");
    return;
}


static void print_one_node(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    // if ((root->parent_node) != NULL)
    // {
    //     return;
    // }
    // printf("\n\nis_it_possible_to_print_a_single_node = %d\n", *is_it_possible_to_print_a_single_node);
    // printf("root information:\n");
    // printf("root parent = %p\n", root->parent_node);
    // printf("root address = %p\n", root);
    // printf("root type = %d\n", (root->value).type);
    // if ((root->value).type == NUMBER)
    // {
    //     printf("root number = %f\n", (root->value).number);
    // }
    // else if ((root->value).type == OPERATION)
    // {
    //     printf("root operation = %d\n", (root->value).operation);
    // }
    // else if ((root->value).type == FUNCTION)
    // {
    //     printf("root function = %d\n", (root->value).function);
    // }
    // else if ((root->value).type == VARIABLE)
    // {
    //     printf("root variable = %d\n", (root->value).variable);
    // }
    if ((root->value).type == NUMBER && *is_it_possible_to_print_a_single_node)
    {
        fprintf(file_pointer, " %f\n", ((root)->value).number);
        *is_it_possible_to_print_a_single_node = false;
    }
    else if ((root->value).type == VARIABLE && *is_it_possible_to_print_a_single_node)
    {
        fprintf(file_pointer, " %s\n", transform_variable_to_str(root->value));
        *is_it_possible_to_print_a_single_node = false;
    }
    return;
}

static void print_mul(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    printf("\n\nroot address in mul = %p\n\n\n", root);
    if ((root == operation_for_print || root->parent_node == operation_for_print) && root == address_of_call)
    {
        fprintf(file_pointer, "\\begin{equation}\n");//% (in print_mul\n");
        // if (((root->left)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, " %f ", ((root->left)->value).number);
        // }
        // if (((root->left)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s ", transform_variable_to_str((root->left)->value));
        // }
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->left in print_mul\n");
        // getchar();
        latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->left);
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        fprintf(file_pointer, "\\cdot");

        // if (((root->right)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, " %f ", ((root->right)->value).number);
        // }
        // if (((root->right)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s ", transform_variable_to_str((root->right)->value));
        // }
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->right in print_mul\n");
        // getchar();
        latex_bypass_of_tree(root->right, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->right);
        *is_it_possible_to_print_an_operation = false;
        //*is_it_possible_to_print_a_single_node = 0;
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->right)->value));
        // }

        fprintf(file_pointer, "\n \\end{equation}\n");//% ( in print_mul\n");
    }
    // fprintf(file_pointer, "\\begin{equation}\n %d \\cdot %d\n \\end{equation}\n",
    //                       ((root->left)->value).number,
    //                       ((root->right)->value).number);
    return;
}

static void print_fraction(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    printf("\n\nroot address in fraction = %p\n\n\n", root);
    if ((root == operation_for_print || root->parent_node == operation_for_print) && root == address_of_call)
    {
        fprintf(file_pointer, "\\begin{equation}\n\\frac{");//% (in print_fraction\n\\frac{");
        // if (((root->left)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, " %f }", ((root->left)->value).number);
        // }
        // else if (((root->left)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s }", transform_variable_to_str((root->left)->value));
        // }
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->left in print_fraction\n");
        // getchar();
        latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->left);
        fprintf(file_pointer, "}{");
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->right in print_fraction\n");
        // getchar();
        latex_bypass_of_tree(root->right, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->right);
        *is_it_possible_to_print_an_operation = false;
        //*is_it_possible_to_print_a_single_node = 0;
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        // if (((root->right)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, "{ %f", ((root->right)->value).number);
        // }
        // else if (((root->right)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, "{ %s", transform_variable_to_str((root->right)->value));
        // }
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        fprintf(file_pointer, "}\n \\end{equation}\n");//% (in print_fraction\n");
    }
    // fprintf(file_pointer, "\\begin{equation}\n\\frac{ %d }{ %d }\n \\end{equation}\n",
    //                       ((root->left)->value).number,
    //                       ((root->right)->value).number);
    return;
}

static void print_pow(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    printf("\n\nroot address in pow = %p\n\n\n", root);
    if ((root == operation_for_print || root->parent_node == operation_for_print) && root == address_of_call)
    {
        fprintf(file_pointer, "\\begin{equation}\n");//% (in print_pow\n");
        // if (((root->left)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, " %f ^", ((root->left)->value).number);
        // }
        // else if (((root->left)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s ^", transform_variable_to_str((root->left)->value));
        // }
        fprintf(file_pointer, "{");
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->left in print_pow\n");
        // getchar();
        latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->left);
        fprintf(file_pointer, "} ^ {");
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->right in print_pow\n");
        // getchar();
        latex_bypass_of_tree(root->right, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->right);
        *is_it_possible_to_print_an_operation = false;
        //*is_it_possible_to_print_a_single_node = 0;
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        // if (((root->right)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, "{ %f }", ((root->right)->value).number);
        // }
        // else if (((root->right)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, "{ %s }", transform_variable_to_str((root->right)->value));
        // }
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        fprintf(file_pointer, "}\n \\end{equation}\n");//% (in print_pow\n");
    }
    // fprintf(file_pointer, "\\begin{equation}\n%d ^ { %d }\n \\end{equation}\n",
    //                       ((root->left)->value).number,
    //                       ((root->right)->value).number);
    return;
}


static void print_add_or_sub(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    printf("\n\nroot address in add_or_sub = %p\n\n\n", root);
    if ((root == operation_for_print || root->parent_node == operation_for_print) && root == address_of_call)
    {
        fprintf(file_pointer, "\\begin{equation}\n");//% (in print_add_or_sub\n");
        // printf("type = %d\n", (root->value).type);
        // printf("operation = %d\n", (root->value).operation);
        // printf("left number = %f\nright_number = %f\n", ((root->left)->value).number, ((root->right)->value).number);
        // if (((root->left)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, " %f ", ((root->left)->value).number);
        // }
        // else if (((root->left)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s ", transform_variable_to_str((root->left)->value));
        // }
        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->left in print_add_or_sub\n");
        // getchar();
        latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->left);
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        fprintf(file_pointer, "%c", (((root->value).operation) == OP_ADD) ? '+' : '-');

        *is_it_possible_to_print_a_single_node = true;
        // printf("go to latex print root->right in print_add_or_sub\n");
        // getchar();
        latex_bypass_of_tree(root->right, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, root->right);
        *is_it_possible_to_print_an_operation = false;
        //*is_it_possible_to_print_a_single_node = 0;
        // if (((root->right)->value).type == NUMBER)
        // {
        //     fprintf(file_pointer, " %f ", ((root->right)->value).number);
        // }
        // else if (((root->right)->value).type == VARIABLE)
        // {
        //     fprintf(file_pointer, " %s ", transform_variable_to_str((root->right)->value));
        // }
        // else if (((root->left)->value).type == OPERATION)
        // {
        //     fprintf(file_pointer, " %c ", transform_operation_to_str((root->left)->value));
        // }

        fprintf(file_pointer, "\n \\end{equation}\n");//% (in print_add_or_sub\n");
    }
    // fprintf(file_pointer, "\\begin{equation}\n %d %c %d\n \\end{equation}\n",
    //                       ((root->left)->value).number,
    //                       (((root->value).operation) == OP_ADD) ? '+' : '-',
    //                       ((root->right)->value).number);
    return;
}


static void analyze_root(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    // printf("\n\nroot information:\n");
    // printf("root parent = %p\n", root->parent_node);
    // printf("root address = %p\n", root);
    // printf("root type = %d\n", (root->value).type);
    // if ((root->value).type == NUMBER)
    // {
    //     printf("root number = %f\n", (root->value).number);
    // }
    // else if ((root->value).type == OPERATION)
    // {
    //     printf("root operation = %d\n", (root->value).operation);
    // }
    // else if ((root->value).type == FUNCTION)
    // {
    //     printf("root function = %d\n", (root->value).function);
    // }
    // else if ((root->value).type == VARIABLE)
    // {
    //     printf("root variable = %d\n", (root->value).variable);
    // }
    //printf("root address = %p\n", root);
    //printf("root operation = %d\n", (root->value).operation);
    if ((root->value).type == OPERATION)
    {
        //printf("root address = %p    root value type = %d\n", root, (root->value).type);
        switch ((root->value).operation)
        {
            case OP_ADD:    print_add_or_sub(root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call); break;
            case OP_SUB:    print_add_or_sub(root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call); break;
            case OP_MUL:    print_mul       (root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call); break;
            case OP_DIV:    print_fraction  (root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call); break;
            case OP_DEG:    print_pow       (root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call); break;
            case NOT_AN_OP:                                       break;
            default: break;
        }
    }
    else if ((root->value).type == FUNCTION)
    {
        for (size_t index = 0; index < size_of_latex_view_functions; index++)
        {
            if ((root->value).function == latex_view_functions[index].name)
            {
                latex_view_functions[index].print_function(root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call);
                break;
            }
        }
    }
    else
    {
        print_one_node(root, file_pointer, is_it_possible_to_print_a_single_node);
    }

    return;
}

static void latex_bypass_of_tree(struct Node *root, FILE *file_pointer, bool *is_it_possible_to_print_a_single_node, bool *is_it_possible_to_print_an_operation, struct Node *operation_for_print, struct Node *address_of_call)
{
    if (root == NULL)
    {
        return;
    }
    // if ((root->value).type == OPERATION && root != operation_for_print)
    // {
    //     *is_it_possible_to_print_an_operation = false;
    // }
    // else if (root->parent_node == operation_for_print)
    // {
    //     *is_it_possible_to_print_an_operation = true;
    // }
    if ((root->value).type == OPERATION)
    {
        *is_it_possible_to_print_a_single_node = false;
    }
    latex_bypass_of_tree(root->left, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call);
    analyze_root(root, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call);
    latex_bypass_of_tree(root->right, file_pointer, is_it_possible_to_print_a_single_node, is_it_possible_to_print_an_operation, operation_for_print, address_of_call);
}


Errors_of_tree create_latex_file(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        return ERROR_OF_LATEX_DUMP;
    }
    fprintf(file_pointer,   "\\documentclass{article}\n"
                            "\\usepackage{graphicx}\n"
                            "\\title{test}\n"
                            "\\author{Zevs Grom}\n"
                            "\\date{January 2025}\n"
                            "\\begin{document}\n");
                            //"\\end{document}\n");
    return NO_ERRORS_TREE;
}

void latex_dump(struct Node *root, FILE *file_pointer, const char *action)
{
    if (root == NULL || file_pointer == NULL)
    {
        return;
    }
    fprintf(file_pointer, "\n%s:", action);
    bool is_it_possible_to_print_a_single_node = false;
    bool is_it_possible_to_print_an_operation = true;
    struct Node *operation_for_print = root;
    struct Node *address_of_call = root;
    if (((root->value).type == NUMBER || (root->value).type == VARIABLE) && root->parent_node == NULL)
    {
        is_it_possible_to_print_a_single_node = true;
    }
    latex_bypass_of_tree(root, file_pointer, &is_it_possible_to_print_a_single_node, &is_it_possible_to_print_an_operation, operation_for_print, address_of_call);
    return;
}

Errors_of_tree close_latex_file(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        return ERROR_OF_LATEX_DUMP;
    }
    fprintf(file_pointer, "\\end{document}\n");
    fclose(file_pointer);
    return NO_ERRORS_TREE;
}
