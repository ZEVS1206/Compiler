#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "tree.h"
#include "tree_input.h"

static void simplyfying_node_with_plus_or_minus(struct Node **root, FILE *file_pointer);
static void simplyfying_node_with_mul(struct Node **root, FILE *file_pointer);
static void simplyfying_node_with_div(struct Node **root, FILE *file_pointer);
static void simplyfying_node_with_pow(struct Node **root, FILE *file_pointer);
static void simplyfying_node_with_sin(struct Node **root, FILE *file_pointer);
static void simplyfying_node_with_cos(struct Node **root, FILE *file_pointer);
static void create_value(struct Value *root_value, struct Value *new_value);
static void destructor_recursive(struct Node *root);
static void calculation_of_subtree(struct Node **root, FILE *file_pointer);
static void calculation_of_nodes(struct Value *left_value, struct Value *right_value, Operations operation, double *result);
static void differentiation_of_constant(struct Node **root, FILE *file_pointer);
static void differentiation_of_variable(struct Node **root, FILE *file_pointer);
static void differentiation_of_operation(struct Node **root, FILE *file_pointer);
static void differentiation_of_function(struct Node **root, FILE *file_pointer);
static void differentiation_sum_sub(struct Node **root, FILE *file_pointer);
static void differentiation_mul(struct Node **root, FILE *file_pointer);
static void differentiation_mul_expressions(struct Node **root, FILE *file_pointer);
static void differentiation_pow_expression(struct Node **root, FILE *file_pointer);
static void differentiation_pow(struct Node **root, FILE *file_pointer);
static void differentiation_div(struct Node **root, FILE *file_pointer);
static void differentiation_of_func_sin(struct Node **root, FILE *file_pointer);
static void differentiation_of_func_cos(struct Node **root, FILE *file_pointer);
static void replace_node_with_number(struct Node **root, double number);
static bool try_differentiation_constant(struct Node **root, FILE *file_pointer);
static bool try_differentiation_variable(struct Node **root, FILE *file_pointer);
static bool try_differentiation_operation(struct Node **root, FILE *file_pointer);
static bool try_differentiation_function(struct Node **root, FILE* file_pointer);
static void try_differentiation_of_constant_and_variable(struct Node **root, FILE *file_pointer);
static bool is_node_number(const struct Node *root, int number);

const struct Function_interface K_functions[] =
{
    {FUNC_SIN, simplyfying_node_with_sin, differentiation_of_func_sin, calculation_of_subtree}
    // {FUNC_COS, simplyfying_node_with_cos, differentiation_of_func_cos, calculation_of_subtree}
};

const size_t size_of_arr_functions = sizeof(K_functions) / sizeof(K_functions[0]);

const struct Operation_interface K_operations[] =
{
    {simplyfying_node_with_plus_or_minus, differentiation_sum_sub,        calculation_of_subtree, OP_ADD},
    {simplyfying_node_with_plus_or_minus, differentiation_sum_sub,        calculation_of_subtree, OP_SUB},
    {simplyfying_node_with_mul,           differentiation_mul,            calculation_of_subtree, OP_MUL},
    {simplyfying_node_with_div,           differentiation_div,            calculation_of_subtree, OP_DIV},
    {simplyfying_node_with_pow,           differentiation_pow_expression, calculation_of_subtree, OP_DEG}
};

const size_t size_of_operations = sizeof(K_operations) / sizeof(K_operations[0]);

static void create_value(struct Value *root_value, struct Value *new_value)
{
    if (root_value == NULL || new_value == NULL)
    {
        return;
    }
    new_value->type = root_value->type;
    switch(new_value->type)
    {
        case VARIABLE:  new_value->variable  = root_value->variable;  break;
        case NUMBER:    new_value->number    = root_value->number;    break;
        case OPERATION: new_value->operation = root_value->operation; break;
        case FUNCTION:  new_value->function  = root_value->function;  break;
    }
    return;
}



struct Node* copy_node(struct Node *root, struct Node *parent)
{
    if (root == NULL)
    {
        return NULL;
    }
    struct Node* new_node = (Node *) calloc(1, sizeof(Node));
    new_node->left = copy_node(root->left, root);
    new_node->right = copy_node(root->right, root);
    new_node->value = root->value;
    new_node->parent_node = parent;
    return new_node;
}



static void replace_node_with_number(struct Node **root, double number)
{
    if ((*root) == NULL)
    {
        return;
    }
    struct Value new_value = {.type = NUMBER, .number = number};
    Errors_of_tree error = create_new_node(root, &new_value, NULL, NULL);
    return;
}

static void try_differentiation_of_constant_and_variable(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    bool attempt = try_differentiation_constant(root, file_pointer);
    if (!attempt)
    {
        attempt = try_differentiation_variable(root, file_pointer);
    }
    return;
}

static bool is_node_number(const struct Node *root, int number)
{
    if (root == NULL)
    {
        return false;
    }
    if ((root->value).type == NUMBER && (root->value).number == number)
    {
        return true;
    }
    return false;
}

static void simplyfying_node_with_sin(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    assert(((*root)->value).type == FUNCTION);
    assert(((*root)->value).function == FUNC_SIN);
    if ((((*root)->left)->value).type == NUMBER)
    {
        return calculation_of_subtree(root, file_pointer);
    }
    return;
}



static void simplyfying_node_with_plus_or_minus(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    assert(((*root)->value).type == OPERATION);
    assert(((*root)->value).operation == OP_ADD || ((*root)->value).operation == OP_SUB);

    if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
    {
        return calculation_of_subtree(root, file_pointer);
    }

    struct Value new_root_value = {};
    Errors_of_tree error = NO_ERRORS_TREE;
    if (is_node_number((*root)->left, 0))
    {
        create_value(&(((*root)->right)->value), &new_root_value);
        error = create_new_node(root, &new_root_value, ((*root)->right)->left, ((*root)->right)->right);
    }
    else if (is_node_number((*root)->right, 0))
    {
        create_value(&(((*root)->left)->value), &new_root_value);
        error = create_new_node(root, &new_root_value, ((*root)->left)->left, ((*root)->left)->right);
    }
}

static void simplyfying_node_with_mul(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    assert(((*root)->value).type == OPERATION);
    assert(((*root)->value).operation == OP_MUL);
    if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
    {
        return calculation_of_subtree(root, file_pointer);
    }

    struct Value new_root_value = {};
    Errors_of_tree error = NO_ERRORS_TREE;
    if (is_node_number((*root)->left, 0) || is_node_number((*root)->right, 0))
    {
        new_root_value.type = NUMBER;
        new_root_value.number = 0;
        error = create_new_node(root, &new_root_value, NULL, NULL);
        return;
    }

    if (is_node_number((*root)->left, 1))
    {
        create_value(&(((*root)->right)->value), &new_root_value);
        error = create_new_node(root, &new_root_value, ((*root)->right)->left, ((*root)->right)->right);
    }
    else if (is_node_number((*root)->right, 1))
    {
        create_value(&(((*root)->left)->value), &new_root_value);
        error = create_new_node(root, &new_root_value, ((*root)->left)->left, ((*root)->left)->right);
    }
    return;
}

static void simplyfying_node_with_div(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    assert(((*root)->value).type == OPERATION);
    assert(((*root)->value).operation == OP_DIV);

    if (is_node_number((*root)->right, 0))
    {
        printf("Error! Division by Zero!\n");
        return;
    }

    if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
    {
        return calculation_of_subtree(root, file_pointer);
    }

    struct Value new_root_value = {};
    Errors_of_tree error = NO_ERRORS_TREE;
    if (is_node_number((*root)->left, 0))
    {
        new_root_value.type = NUMBER;
        new_root_value.number = 0;
        error = create_new_node(root, &new_root_value, NULL, NULL);
        return;
    }

    if (is_node_number((*root)->right, 1))
    {
        create_value(&(((*root)->left)->value), &new_root_value);
        error = create_new_node(root, &new_root_value, ((*root)->left)->left, ((*root)->left)->right);
    }
    return;
}

static void simplyfying_node_with_pow(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    assert(((*root)->value).type == OPERATION);
    assert(((*root)->value).operation == OP_DEG);

    if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
    {
        return calculation_of_subtree(root, file_pointer);
    }

    struct Value new_root_value = {};
    Errors_of_tree error = NO_ERRORS_TREE;


    if (is_node_number((*root)->right, 0))
    {
        new_root_value.type = NUMBER;
        new_root_value.number = 1;
        error = create_new_node(root, &new_root_value, NULL, NULL);
        return;
    }

    if (is_node_number((*root)->right, 1))
    {
        create_value(&(((*root)->left)->value), &new_root_value);
        error = create_new_node(root, &new_root_value, ((*root)->left)->left, ((*root)->left)->right);
    }
    return;
}



void symplifying_tree(struct Node **root, FILE *file_pointer)
{
    if (*root == NULL)
    {
        return;
    }
    //latex_dump(*root, file_pointer, "Let's simplyfy this statement");
    symplifying_tree(&((*root)->left), file_pointer);
    symplifying_tree(&((*root)->right), file_pointer);
    if (((*root)->value).type == OPERATION)
    {
        for (size_t index = 0; index < size_of_operations; index++)
        {
            if (K_operations[index].operation_type == ((*root)->value).operation)
            {
                K_operations[index].symplifying_tree(root, file_pointer);
                break;
            }
        }
        // if (((*root)->value).operation == OP_ADD)
        // {
        //     simplyfying_node_with_plus_or_minus(root);
        // }
        // else if ((((*root)->left)->value).type != NUMBER || (((*root)->right)->value).type != NUMBER)
        // {
        //     simplyfying_node_with_variable(root);
        // }
        // else if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
        // {
        //     calculation_of_subtree(root);
        // }
    }
    else if (((*root)->value).type == FUNCTION)
    {
        for (size_t index = 0; index < size_of_arr_functions; index++)
        {
            if (K_functions[index].function_name == ((*root)->value).function)
            {
                K_functions[index].symplifying_tree(root, file_pointer);
                break;
            }
        }
    }
    return;
}



static void differentiation_of_func_sin(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    struct Node *old_left = copy_node((*root)->left, *root);
    differentiation((*root)->left, file_pointer);
    struct Node *differentiation_left = copy_node((*root)->left, *root);
    struct Value new_root_value = {.type = OPERATION, .operation = OP_MUL};
    struct Value new_root_left_value = {.type = FUNCTION, .function = FUNC_COS};
    Errors_of_tree error = NO_ERRORS_TREE;
    struct Node *new_function = NULL;
    error = create_new_node(&new_function, &new_root_left_value, old_left, NULL);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(root, &new_root_value, new_function, differentiation_left);
    return;
}

static void differentiation_div(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }

    struct Node *old_left = copy_node((*root)->left, *root);
    struct Node *old_right = copy_node((*root)->right, *root);
    struct Node *denominator_node_left = copy_node((*root)->right, *root);
    struct Node *denominator_node_right = copy_node((*root)->right, *root);
    differentiation((*root)->left, file_pointer);
    differentiation((*root)->right, file_pointer);
    struct Node *diffirentiation_left = copy_node((*root)->left, *root);
    struct Node *diffirentiation_right = copy_node((*root)->right, *root);
    struct Value new_root_value = {.type = OPERATION, .operation = OP_DIV};
    struct Value new_numerator_value = {.type = OPERATION, .operation = OP_SUB};
    struct Value new_denominator_value = {.type = OPERATION, .operation = OP_MUL};
    struct Value new_numerator_left_and_right_value = {.type = OPERATION, .operation = OP_MUL};
    struct Node *new_numerator_left_node = NULL;
    struct Node *new_numerator_right_node = NULL;
    struct Node *new_numerator_node = NULL;
    struct Node *new_denominator_node = NULL;
    Errors_of_tree error = NO_ERRORS_TREE;
    error = create_new_node(&new_numerator_left_node, &new_numerator_left_and_right_value, diffirentiation_left, old_right);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&new_numerator_right_node, &new_numerator_left_and_right_value, old_left, diffirentiation_right);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&new_numerator_node, &new_numerator_value, new_numerator_left_node, new_numerator_right_node);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&new_denominator_node, &new_denominator_value, denominator_node_left, denominator_node_right);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(root, &new_root_value, new_numerator_node, new_denominator_node);
    return;
}

static void differentiation_sum_sub(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    bool attempt = true;
    if ((((*root)->left)->value).type == OPERATION)
    {
        attempt = try_differentiation_operation(&((*root)->left), file_pointer);
    }
    else
    {
        try_differentiation_of_constant_and_variable(&((*root)->left), file_pointer);
    }

    if ((((*root)->right)->value).type == OPERATION)
    {
        attempt = try_differentiation_operation(&((*root)->right), file_pointer);
    }
    else
    {
        try_differentiation_of_constant_and_variable(&((*root)->right), file_pointer);
    }
    //calculation_of_subtree(root);
}

static void differentiation_mul_expressions(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    Errors_of_tree error = NO_ERRORS_TREE;
    struct Node *old_left = copy_node((*root)->left, *root);
    struct Node *old_right = copy_node((*root)->right, *root);
    differentiation(((*root)->left), file_pointer);
    differentiation(((*root)->right), file_pointer);
    struct Node *diffirentiation_left = copy_node((*root)->left, *root);
    struct Node *diffirentiation_right = copy_node((*root)->right, *root);
    struct Value new_root_value = {.type = OPERATION, .operation = OP_ADD};
    struct Value new_nodes_value = {.type = OPERATION, .operation = OP_MUL};
    struct Node *new_left_node = NULL;
    struct Node *new_right_node = NULL;
    error = create_new_node(&(new_left_node), &new_nodes_value, diffirentiation_left, old_right);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&(new_right_node), &new_nodes_value, old_left, diffirentiation_right);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(root, &new_root_value, new_left_node, new_right_node);
    return;
}

static void differentiation_pow(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }

    if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
    {
        calculation_of_subtree(root, file_pointer);
        bool attempt = try_differentiation_constant(root, file_pointer);
        return;
    }

    differentiation_pow_expression(root, file_pointer);
    return;
}


static void differentiation_pow_expression(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    Errors_of_tree error = NO_ERRORS_TREE;
    struct Value new_root_value = {.type = OPERATION, .operation = OP_MUL};
    struct Value new_left_node_value = {.type = NUMBER, .number = (((*root)->right)->value).number};
    struct Value new_right_node_value = {.type = OPERATION, .operation = OP_DEG};
    struct Value new_right_right_node_value = {.type = NUMBER, .number = (((*root)->right)->value).number - 1};
    struct Value new_right_left_node_value = {.type = VARIABLE, .variable = (((*root)->left)->value).variable};
    struct Node *new_left_node = NULL;
    struct Node *new_right_node = NULL;
    struct Node *new_right_left_node = NULL;
    struct Node *new_right_right_node = NULL;
    error = create_new_node(&new_left_node, &new_left_node_value, NULL, NULL);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&new_right_left_node, &new_right_left_node_value, NULL, NULL);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&new_right_right_node, &new_right_right_node_value, NULL, NULL);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(&new_right_node, &new_right_node_value, new_right_left_node, new_right_right_node);
    if (error != NO_ERRORS_TREE)
    {
        return;
    }
    error = create_new_node(root, &new_root_value, new_left_node, new_right_node);
    return;
}




static void differentiation_mul(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    // if ((((*root)->left)->value).type == NUMBER || (((*root)->right)->value).type == NUMBER)
    // {
    //     differentiation_mul_expression_and_constant(root);
    // }
    // else if (((((*root)->left)->value).type == VARIABLE && (((*root)->right)->value).type == VARIABLE) ||
    //          (((*root)->left)->value).type == OPERATION || (((*root)->right)->value).type == OPERATION)
    // {
    differentiation_mul_expressions(root, file_pointer);
    // else if ((((*root)->left)->value).type == VARIABLE && (((*root)->right)->value).type == NUMBER ||
    //          (((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == VARIABLE)
    // {
    //     differentiation_mul_variable_and_constant(root);
    // }
    if ((((*root)->left)->value).type == NUMBER && (((*root)->right)->value).type == NUMBER)
    {
        calculation_of_subtree(root, file_pointer);
        bool attempt = try_differentiation_constant(root, file_pointer);
    }

    return;
}

static void differentiation_of_function(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }

    for(size_t index = 0; index < size_of_arr_functions; index++)
    {
        if (K_functions[index].function_name == ((*root)->value).function)
        {
            K_functions[index].differentiation(root, file_pointer);
            break;
        }
    }
    return;
}


static void differentiation_of_operation(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    for (size_t index = 0; index < size_of_operations; index++)
    {
        if (K_operations[index].operation_type == ((*root)->value).operation)
        {
            K_operations[index].differentiation(root, file_pointer);
            break;
        }
    }
    // switch (((*root)->value).operation)
    // {
    //     case OP_ADD: differentiation_sum_sub  (root); break;
    //     case OP_SUB: differentiation_sum_sub  (root); break;
    //     case OP_MUL: differentiation_mul      (root); break;
    //     case OP_DIV: differentiation_div      (root); break;
    //     case OP_DEG: differentiation_pow      (root); break;
    //     default: break;
    // }
    return;
}

static bool try_differentiation_constant(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return false;
    }
    if (((*root)->value).type == NUMBER)
    {
        differentiation_of_constant(root, file_pointer);
        return true;
    }
    return false;
}

static bool try_differentiation_variable(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return false;
    }
    if (((*root)->value).type == VARIABLE)
    {
        differentiation_of_variable(root, file_pointer);
        return true;
    }
    return false;
}

static bool try_differentiation_operation(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return false;
    }
    if (((*root)->value).type == OPERATION)
    {
        differentiation_of_operation(root, file_pointer);
        return true;
    }
    return false;
}

static bool try_differentiation_function(struct Node **root, FILE* file_pointer)
{
    if ((*root) == NULL)
    {
        return false;
    }
    if (((*root)->value).type == FUNCTION)
    {
        differentiation_of_function(root, file_pointer);
        return true;
    }
    return false;
}

static void differentiation_of_constant(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    if (((*root)->value).type == NUMBER)
    {
        replace_node_with_number(root, 0);
    }
    return;
}

static void differentiation_of_variable(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    if (((*root)->value).type == VARIABLE)
    {
        replace_node_with_number(root, 1);
    }
    return;
}


void differentiation(struct Node *root, FILE *file_pointer)
{
    if (root == NULL)
    {
        return;
    }
    bool attempt = try_differentiation_constant(&root, file_pointer);
    if (attempt)
    {
        return;
    }
    attempt = try_differentiation_variable(&root, file_pointer);
    if (attempt)
    {
        return;
    }
    attempt = try_differentiation_operation(&root, file_pointer);
    if (attempt)
    {
        return;
    }
    attempt = try_differentiation_function(&root, file_pointer);
}

static void calculation_of_nodes(struct Value *left_value, struct Value *right_value, Operations operation, double *result)
{
    if (left_value == NULL || right_value == NULL)
    {
        return;
    }
    switch (operation)
    {
        case OP_ADD: *result = left_value->number + right_value->number; break;
        case OP_SUB: *result = left_value->number - right_value->number; break;
        case OP_MUL: *result = left_value->number * right_value->number; break;
        case OP_DIV: *result = left_value->number / right_value->number; break;
        case OP_DEG: *result = (int)pow(left_value->number, right_value->number);
        default: break;
    }
    return;
}


static void calculation_of_subtree(struct Node **root, FILE *file_pointer)
{
    if ((*root) == NULL)
    {
        return;
    }
    if (((*root)->value).type == OPERATION)
    {
        if ((*root)->left != NULL && (*root)->right != NULL && (((*root)->left )->value).type == NUMBER
                                                            && (((*root)->right)->value).type == NUMBER)
        {
            double result = 0;
            calculation_of_nodes(&(((*root)->left)->value), &(((*root)->right)->value), ((*root)->value).operation, &result);
            replace_node_with_number(root, result);
        }
    }
    else if (((*root)->value).type == FUNCTION)
    {
        double result = 0;
        for (size_t index = 0; index < size_of_functions; index++)
        {
            if (G_functions[index].function_name == ((*root)->value).function)
            {
                result = G_functions[index].proccess_function((((*root)->left)->value).number);
                break;
            }
        }
        replace_node_with_number(root, result);
    }
    return;
}

void calculation_of_tree(struct Node *root, FILE *file_pointer)
{
    if (root == NULL)
    {
        return;
    }
    calculation_of_tree(root->left, file_pointer);
    calculation_of_tree(root->right, file_pointer);
    calculation_of_subtree(&root, file_pointer);
}



Errors_of_tree create_new_node(struct Node **root, struct Value *value, struct Node *left, struct Node *right)
{
    if (*root == NULL)
    {
        *root = (Node *) calloc(1, sizeof(Node));
        if (*root == NULL)
        {
            return ERROR_OF_PUSH;
        }
    }
    (*root)->is_processed = false;
    (*root)->value = *value;
    // (*root)->left = left;
    // (*root)->right = right;
    if ((*root)->left != NULL && (*root)->left != left)
    {
        // printf("left = %p\n", left);
        // printf("root = %p\n", *root);
        free((*root)->left);
        (*root)->left = left;
        if (left != NULL)
        {
            ((*root)->left)->parent_node = *root;
        }
    }
    else
    {
        (*root)->left = left;
        if (left != NULL)
        {
            ((*root)->left)->parent_node = *root;
        }
    }
    if ((*root)->right != NULL && (*root)->right != right)
    {

        free((*root)->right);
        (*root)->right = right;
        if (right != NULL)
        {
            ((*root)->right)->parent_node = *root;
        }
    }
    else
    {
        (*root)->right = right;
        if (right != NULL)
        {
            ((*root)->right)->parent_node = *root;
        }
    }
    return NO_ERRORS_TREE;
}


Errors_of_tree tree_constructor(struct Tree *tree)
{
    if (tree == NULL)
    {
        return ERROR_OF_CONSTRUCTOR;
    }
    //tree->interface = (struct List_of_operations *) calloc(1, sizeof(struct List_of_operations));
    // if (tree->interface == NULL)
    // {
    //     return ERROR_OF_CONSTRUCTOR;
    // }
    // (tree->interface)->size_of_operations = size_of_all_operations;
    // (tree->interface)->operations = (struct Operation_interface *) calloc((tree->interface)->size_of_operations, sizeof(struct Operation_interface));
    // if ((tree->interface)->operations == NULL)
    // {
    //     return ERROR_OF_CONSTRUCTOR;
    // }
    tree->error = NO_ERRORS_TREE;
    tree->root = (Node *) calloc(1, sizeof(Node));
    if (tree->root == NULL)
    {
        return ERROR_OF_CONSTRUCTOR;
    }
    tree->tmp_root = tree->root;
    return NO_ERRORS_TREE;
}

Errors_of_tree tree_destructor(struct Tree *tree)
{
    if (tree == NULL)
    {
        return ERROR_OF_DESTRUCTOR;
    }
    destructor_recursive(tree->root);
    if (tree->error != NO_ERRORS_TREE)
    {
        return tree->error;
    }
    return NO_ERRORS_TREE;
}


static void destructor_recursive(struct Node *root)
{
    if (root == NULL)
    {
        return;
    }

    destructor_recursive(root->left);
    destructor_recursive(root->right);
    destructor_recursive(root->node_after_operator);
    destructor_recursive(root->node_for_operator_else);
    //printf("Here\n");
    free(root);
    root = NULL;
}
