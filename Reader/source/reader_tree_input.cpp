#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "tree.h"
#include "tree_input.h"
#include "tree_tex_dump.h"
//#include "../../Assembler/include/assembler.h"

const char *built_in_functions[] = {"print", "input"};
const size_t size_of_built_in_functions = sizeof(built_in_functions) / sizeof(char *);
bool are_any_functions = false;

//static void parse_information_from_file_by_staples(struct Node *root, char **buffer, char *end_pointer);
static bool isbracket(const char symbol);
static void parse_information_from_file_to_lexical_analyze_array(struct Node **lexical_analyze_array, char **buffer, char *end_pointer, int *len_of_lexical_analyze_array);
//static bool parse_operation_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array);
static void parse_function_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array, Value_type type);
static bool parse_operator_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array);
static bool parse_comp_operation_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array);
static void parse_variable_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array);
static void parse_number_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array);
static void parse_information_from_lexical_analyze_array_by_recursive_descent(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index);
static void get_number(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_variable(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_definition_of_function(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_staples_expression_or_number_or_variable(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_expression_with_plus_or_minus(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_expression_with_mul_or_div(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_expression_with_pow(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_expression_with_comparison_operations(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_operator(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_assignment_operator(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_function(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_functions_parametres(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_assignment_operator_or_function(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_if_or_while_operator(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_operator_return(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static void get_operator_else(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent);
static size_t get_size_of_file(FILE *file_pointer);
static char * get_value_from_file(char *str, size_t size_of_str, char *buffer, char *end_pointer);
static char * get_operation_from_file(char *str, size_t size_of_str, char *buffer, char *end_pointer);
static char * skip_spaces(char *buffer, char *end_pointer);
static void transform_to_variable(const char *str, struct Value *value);
static void transform_to_comparison_operation(const char *str, struct Value *value);
static void bypass_of_tree(struct Node *root);
static void print_lexical_analyze_array(struct Node *lexical_analyze_array, size_t size);
static bool is_comparison_operation(const char *str);

static bool isbracket(const char symbol)
{
    if (symbol == '(' ||
        symbol == ')' ||
        symbol == '{' ||
        symbol == '}' ||
        symbol == '[' ||
        symbol == ']')
        {
            return true;
        }
    return false;
}


static void print_lexical_analyze_array(struct Node *lexical_analyze_array, size_t size)
{
    for (size_t index = 0; index < size; index++)
    {
        printf("\n\nnew root:\n");
        printf("position = %d\n", index);
        printf("type = %d\n", (lexical_analyze_array[index].value).type);
        if ((lexical_analyze_array[index].value).type == NUMBER)
        {
            printf("number = %f\n", (lexical_analyze_array[index].value).number);
        }
        else if ((lexical_analyze_array[index].value).type == OPERATION)
        {
            printf("operation = %d\n", (lexical_analyze_array[index].value).operation);
        }
        else if ((lexical_analyze_array[index].value).type == OPERATOR)
        {
            printf("operator = %d\n", (lexical_analyze_array[index].value).operator_);
        }
        else if ((lexical_analyze_array[index].value).type == COMP_OPERATION)
        {
            printf("comp_operation = %d\n", (lexical_analyze_array[index].value).comp_operation);
        }
        else if ((lexical_analyze_array[index].value).type == VARIABLE)
        {
            printf("variable = %s\n", (lexical_analyze_array[index].value).variable_name);
        }
        else if ((lexical_analyze_array[index].value).type == FUNCTION ||
                 (lexical_analyze_array[index].value).type == BUILT_IN_FUNCTION ||
                 (lexical_analyze_array[index].value).type == CALLER_OF_FUNCTION)
        {
            printf("function = %s\n", (lexical_analyze_array[index].value).function_name);
        }
    }
    return;
}




static void bypass_of_tree(struct Node *root)
{
    if (root == NULL)
    {
        return;
    }
    bypass_of_tree(root->left);
    bypass_of_tree(root->right);
    //printf("root->value.type = %d\n", (root->value).type);
    if (root->left != NULL)
    {
        (root->left)->parent_node = root;
    }
    if (root->right != NULL)
    {
        (root->right)->parent_node = root;
    }
    return;
}

static char * skip_spaces(char *buffer, char *end_pointer)
{
    if (buffer == end_pointer)
    {
        return buffer;
    }
    while (buffer < end_pointer && isspace(*buffer))
    {
        buffer++;
    }
    return buffer;
}


static void transform_to_variable(const char *str, struct Value *value)
{
    if (value == NULL || str == NULL)
    {
        return;
    }

    bool is_variable = true;
    for (int index = 0; str[index] != '\0'; index++)
    {
        if (!isalpha(str[index]) && !isdigit(str[index]))
        {
            is_variable = false;
            break;
        }
    }
    if (is_variable)
    {
        value->variable = VAR;
        value->type = VARIABLE;
    }
    else
    {
        value->variable = NOT_A_VAR;
    }
    return;
}


void transform_to_arithmetic_operation(const char symbol, struct Value *value)
{
    if (value == NULL)
    {
        return;
    }
    switch (symbol)
    {
        case '+':value->operation = OP_ADD; break;
        case '-':value->operation = OP_SUB; break;
        case '*':value->operation = OP_MUL; break;
        case '/':value->operation = OP_DIV; break;
        case '^':value->operation = OP_DEG; break;
        default: value->operation = NOT_AN_OP; break;
    }
    if (value->operation != NOT_AN_OP)
    {
        value->type = OPERATION;
    }
    return;
}

static void transform_to_comparison_operation(const char *str, struct Value *value)
{
    if (value == NULL)
    {
        return;
    }
    size_t len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    len--;
    //printf("len of str = %lu\n", len);
    if (len == 2 && str[0] == '=')
    {
        value->comp_operation = OP_EQUAL;
    }
    else if (len == 1 && str[0] == '>')//strcasecmp(str, ">") == 0)
    {
        value->comp_operation = OP_MORE;
    }
    else if (len == 1 && str[0] == '<')//strcasecmp(str, "<") == 0)
    {
        value->comp_operation = OP_LESS;
    }
    else if (len == 2 && str[0] == '>')//strcasecmp(str, ">=") == 0)
    {
        value->comp_operation = OP_MORE_OR_EQUAL;
    }
    else if (len == 2 && str[0] == '<')//strcasecmp(str, "<=") == 0)
    {
        value->comp_operation = OP_LESS_OR_EQUAL;
    }
    else if (len == 2 && str[0] == '!')//strcasecmp(str, "!=") == 0)
    {
        value->comp_operation = OP_NOT_EQUAL;
    }
    else
    {
        value->comp_operation = NOT_A_COMP_OP;
    }
    if (value->comp_operation != NOT_A_COMP_OP)
    {
        value->type = COMP_OPERATION;
    }
    return;
}

static bool is_comparison_operation(const char *str)
{
    struct Value value = {};
    //printf("str = %s\n", str);
    transform_to_comparison_operation(str, &value);
    if (value.comp_operation != NOT_A_COMP_OP)
    {
        return true;
    }
    return false;
}

static size_t get_size_of_file(FILE *file_pointer)
{
    fseek(file_pointer, 0L, SEEK_END);
    size_t size_of_file = (size_t)ftell(file_pointer);
    fseek(file_pointer, 0L, SEEK_SET);
    return size_of_file;
}

Errors_of_tree get_tree_from_file(struct Tree *tree, const char *file_source_name)
{
    if (tree == NULL)
    {
        return ERROR_OF_READ_TREE;
    }

    FILE *file_pointer = fopen(file_source_name, "rb");
    if (file_pointer == NULL)
    {
        return ERROR_OF_READ_TREE;
    }

    size_t size_of_file = get_size_of_file(file_pointer);
    char *buffer = (char *)calloc(size_of_file, sizeof(char));
    struct Node *lexical_analyze_array = (struct Node *) calloc(size_of_file, sizeof(struct Node));
    if (buffer == NULL)
    {
        return ERROR_OF_READ_TREE;
    }
    if (lexical_analyze_array == NULL)
    {
        return ERROR_OF_READ_TREE;
    }

    char *end_pointer = &(buffer[size_of_file - 1]);
    char *old_buffer = &(buffer[0]);
    struct Node *old_pointer = &(lexical_analyze_array[0]);

    size_t result_of_reading = fread(buffer, sizeof(char), size_of_file, file_pointer);
    if (result_of_reading != size_of_file)
    {
        return ERROR_OF_READ_FROM_FILE;
    }
    fclose(file_pointer);

    tree->tmp_root = tree->root;
    int index = 0;
    int len_of_lexical_analyze_array = 0;
    parse_information_from_file_to_lexical_analyze_array(&lexical_analyze_array, &buffer, end_pointer, &len_of_lexical_analyze_array);
    //lexical_analyze_array--;
    // printf("last position:\ntype = %d\n", ((lexical_analyze_array[2]).value).type);
    struct Node *new_end_pointer = &(lexical_analyze_array[0]);
    lexical_analyze_array = old_pointer;
    ON_DEBUG(("last position:\ntype = %d\n", ((lexical_analyze_array[2]).value).type);)
    ON_DEBUG(("len of lexical analyze array = %d\n", len_of_lexical_analyze_array);)
    ON_DEBUG(print_lexical_analyze_array(old_pointer, len_of_lexical_analyze_array);)
    parse_information_from_lexical_analyze_array_by_recursive_descent(&(tree->root),lexical_analyze_array, len_of_lexical_analyze_array, &index);
    tree->are_any_functions = are_any_functions;

    free(old_buffer);
    free(old_pointer);
    return NO_ERRORS_TREE;
}

static char * get_value_from_file(char *str, size_t size_of_str, char *buffer, char *end_pointer)
{
    size_t index = 0;
    while (buffer < end_pointer && (isalnum(*buffer) || *buffer == '.' || *buffer == '_') && index < size_of_str && *buffer != ';')
    {
        str[index] = *buffer;
        index++;
        buffer++;
    }
    if (index < size_of_str - 1)
    {
        str[index] = '\0';
    }
    return buffer;
}

static char * get_operation_from_file(char *str, size_t size_of_str, char *buffer, char *end_pointer)
{
    size_t index = 0;
    if (isbracket(*buffer))
    {
        str[index] = *buffer;
        str[index + 1] = '\0';
        buffer++;
        return buffer;
    }
    while (buffer < end_pointer && (!isalnum(*buffer) && *buffer != ';' && !isspace(*buffer)) && index < size_of_str)
    {
        str[index] = *buffer;
        index++;
        buffer++;
        if (isbracket(*buffer))
        {
            break;
        }
    }
    if (index < size_of_str - 1)
    {
        str[index] = '\0';
    }
    return buffer;
}

static void get_functions_parametres(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    while (((lexical_analyze_array[*index]).value).operator_ == OPERATOR_COMMA)
    {
        struct Value new_node_value = (lexical_analyze_array[*index]).value;
        (*index)++;
        ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_functions_parametres\n");)
        ON_DEBUG(getchar();)
        get_staples_expression_or_number_or_variable(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        ON_DEBUG(printf("go to get_functions_parametres from get_functions_parametres\n");)
        ON_DEBUG(getchar();)
        get_functions_parametres(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        if (error != NO_ERRORS_TREE)
        {
            fprintf(stderr, "error = %d\n", error);
            abort();
        }
    }
    //(*index)++;
    return;
}

static void get_function(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Value new_node_value = (lexical_analyze_array[*index]).value;
    (*index)++;
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ROUND_BRACKET_OPEN)
    {
        fprintf(stderr, "Syntax Error! There is no round bracket after function\n");
        abort();
    }
    (*index)++;
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_function\n");)
    ON_DEBUG(getchar();)
    get_staples_expression_or_number_or_variable(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    ON_DEBUG(printf("go to get_functions_parametres from get_function\n");)
    ON_DEBUG(getchar();)
    get_functions_parametres(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ROUND_BRACKET_CLOSE)
    {
        fprintf(stderr, "Syntax Error! There is no round bracket after function parametres\n");
        abort();
    }
    (*index)++;
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    ON_DEBUG(printf("index in get_function = %d\n", *index);)
    ON_DEBUG(getchar();)
    return;
}

static void get_definition_of_function(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    are_any_functions = true;
    struct Value new_node_value = (lexical_analyze_array[*index]).value;
    (*index)++;
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    struct Node *node_after_operator = NULL;
    ON_DEBUG(printf("go to get_function from get_definition_of_function\n");)
    ON_DEBUG(getchar();)
    get_function(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_CURLY_BRACKET_OPEN)
    {
        fprintf(stderr, "Syntax Error! There is no curly bracket before the function main body\n");
        abort();
    }
    (*index)++;
    ON_DEBUG(printf("go to get_operator from get_definition_of_function\n");)
    ON_DEBUG(getchar();)
    get_operator(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_CURLY_BRACKET_CLOSE)
    {
        fprintf(stderr, "Syntax Error! There is no curly bracket after the function main body\n");
        abort();
    }
    (*index)++;
    ON_DEBUG(printf("go to get_operator after function from get_definition_of_function\n");)
    ON_DEBUG(getchar();)
    get_operator(&node_after_operator, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
    (*root)->node_after_operator = node_after_operator;
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    if ((node_after_operator->value).type != OPERATOR || (node_after_operator->value).operator_ != OPERATOR_DEF)
    {
        (*root)->is_last_function = true;
    }
    ON_DEBUG(printf("index in get_definition_of_function = %d\n", *index);)
    ON_DEBUG(getchar();)
    return;
}


static void get_assignment_operator_or_function(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    if (((lexical_analyze_array[*index]).value).type == OPERATOR &&
        ((lexical_analyze_array[*index]).value).operator_ == OPERATOR_DEF)
    {
        ON_DEBUG(printf("go to get_definition_of_function from get_assignment_operator_or_function\n");)
        ON_DEBUG(getchar();)
        get_definition_of_function(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
        return;
    }
    if (((lexical_analyze_array[*index]).value).type == CALLER_OF_FUNCTION ||
        ((lexical_analyze_array[*index]).value).type == BUILT_IN_FUNCTION)
    {
        ON_DEBUG(printf("go to get_function from get_assignment_operator_or_function\n");)
        ON_DEBUG(getchar();)
        get_function(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
        return;
    }
    ON_DEBUG(printf("go to get_assignment_operator from get_assignment_operator_or_function\n");)
    ON_DEBUG(getchar();)
    get_assignment_operator(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
    return;
}

static void get_assignment_operator(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_variable from get_assignment_operator\n");)
    ON_DEBUG(getchar();)
    get_variable(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    if (left_node == NULL)
    {
        return;
    }
    if (((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ASSIGNMENT)
    {
        fprintf(stderr, "Syntax Error! There is no symbol of assignment\n");
        abort();
    }
    struct Value new_node_value = (lexical_analyze_array[*index]).value;
    (*index)++;
    if (((lexical_analyze_array[*index]).value).type == CALLER_OF_FUNCTION ||
        ((lexical_analyze_array[*index]).value).type == BUILT_IN_FUNCTION)
    {
        ON_DEBUG(printf("go to get_function from get_assignment_operator\n");)
        ON_DEBUG(getchar();)
        get_function(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    }
    else
    {
        ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_assignment_operator\n");)
        ON_DEBUG(getchar();)
        get_staples_expression_or_number_or_variable(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    }
    //get_expression_with_comparison_operations(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    //get_staples_expression_or_number_or_variable(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    // if (*buffer[0] != ';')
    // {
    //     syntax_error(buffer, index);
    // }
    // else
    // {
    //     (*buffer)++;
    // }
    ON_DEBUG(printf("index in get_assignment_operator = %d\n", *index);)
    return;
}

static void get_if_or_while_operator(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    // if (*buffer[0] != '(')
    // {
    //     syntax_error(buffer, index);
    // }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    struct Node *node_after_operator = NULL;
    struct Node *node_for_operator_else = NULL;
    struct Value new_node_value = (lexical_analyze_array[*index]).value;
    (*index)++;
    if (((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ROUND_BRACKET_OPEN)
    {
        fprintf(stderr, "Syntax Error! There is no round bracket after if\n");
        abort();
    }
    (*index)++;
    //get_expression_with_plus_or_minus(lexical_analyze_array, buffer, end_pointer, index);
    ON_DEBUG(printf("go to get_expression_with_comparison_operations from get_if_or_while_operator\n");)
    ON_DEBUG(getchar();)
    get_expression_with_comparison_operations(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    if (((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ROUND_BRACKET_CLOSE)
    {
        fprintf(stderr, "Syntax Error! There is no close round bracket after condition of if\n");
        abort();
    }
    (*index)++;
    // if (*buffer[0] != ')')
    // {
    //     syntax_error(buffer, index);
    // }
    // if (*buffer[0] != '{')
    // {
    //     syntax_error(buffer, index);
    // }
    if (((lexical_analyze_array[*index]).value).operator_ != OPERATOR_CURLY_BRACKET_OPEN)
    {
        fprintf(stderr, "Syntax Error! There is no open curly bracket before the main body of if\n");
        abort();
    }
    (*index)++;
    ON_DEBUG(printf("go to get_operator from get_if_or_while_operator\n");)
    ON_DEBUG(getchar();)
    get_operator(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    //printf("*buffer[0] = %c\n", *buffer[0]);
    // if (*buffer[0] != '}')
    // {
    //     syntax_error(buffer, index);
    // }
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_CURLY_BRACKET_CLOSE)
    {
        fprintf(stderr, "Syntax Error! There is no close curly bracket after the main body of if\n");
        abort();
    }
    (*index)++;
    ON_DEBUG(printf("go to get_operator_else from get_if_or_while_operator\n");)
    ON_DEBUG(getchar();)
    get_operator_else(&node_for_operator_else, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    ON_DEBUG(printf("go to get_operator after operator from get_if_or_while_operator\n");)
    ON_DEBUG(getchar();)
    get_operator(&node_after_operator, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
    (*root)->node_after_operator = node_after_operator;
    (*root)->node_for_operator_else = node_for_operator_else;
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    //printf("*buffer[0] = %c\n", *buffer[0]);
    ON_DEBUG(printf("index in get_if_or_while_operator = %d\n", *index);)
    return;
}

static void get_operator_else(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ELSE)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    struct Value new_node_value = (lexical_analyze_array[*index]).value;
    (*index)++;
    if (((lexical_analyze_array[*index]).value).operator_ != OPERATOR_CURLY_BRACKET_OPEN)
    {
        fprintf(stderr, "Syntax Error! There is no open curly bracket before the main body of else\n");
        abort();
    }
    (*index)++;
    ON_DEBUG(printf("go to get_operator from get_operator_else\n");)
    ON_DEBUG(getchar();)
    get_operator(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    if (((lexical_analyze_array[*index]).value).type != OPERATOR ||
        ((lexical_analyze_array[*index]).value).operator_ != OPERATOR_CURLY_BRACKET_CLOSE)
    {
        fprintf(stderr, "Syntax Error! There is no close curly bracket after the main body of else\n");
        abort();
    }
    (*index)++;
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    ON_DEBUG(printf("index in get_operator_else = %d\n", *index);)
    return;
}

static void get_operator_return(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node  = NULL;
    struct Node *right_node = NULL;
    struct Value new_node_value = (lexical_analyze_array[*index]).value;
    (*index)++;
    ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_operator_return\n");)
    ON_DEBUG(getchar();)
    get_staples_expression_or_number_or_variable(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    ON_DEBUG(printf("go to get_operator from get_operator_return\n");)
    ON_DEBUG(getchar();)
    get_operator(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    ON_DEBUG(printf("index in get_operator_return = %d\n", *index);)
    return;
}


static void get_operator(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node  = NULL;
    struct Node *right_node = NULL;
    struct Node *node_after_operator = NULL;
    //get_operator(&left_node, lexical_analyze_array, end_pointer, index, *root);
    // (*index)++;
    // (*lexical_analyze_array)++;
    
    if (((lexical_analyze_array[*index]).value).type == OPERATOR && 
        (((lexical_analyze_array[*index]).value).operator_ == OPERATOR_IF ||
         ((lexical_analyze_array[*index]).value).operator_ == OPERATOR_WHILE))
    {
        ON_DEBUG(printf("go to get_if_or_while_operator from get_operator\n");)
        ON_DEBUG(getchar();)
        get_if_or_while_operator(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
        return;
    }
    else
    {
        if (((lexical_analyze_array[*index]).value).type == OPERATOR && 
        ((lexical_analyze_array[*index]).value).operator_ == OPERATOR_RETURN)
        {
            ON_DEBUG(printf("go to get_operator_return from get_operator\n");)
            ON_DEBUG(getchar();)
            get_operator_return(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
            return;
        }
        else
        {
            ON_DEBUG(printf("go to get_assignment_operator_or_function from get_operator\n");)
            ON_DEBUG(getchar();)
            //get_assignment_operator(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
            get_assignment_operator_or_function(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
            if (left_node != NULL && 
                (left_node->value).type == OPERATOR && 
                (left_node->value).operator_ == OPERATOR_DEF)
            {
                if (*root != NULL)
                {
                    free(*root);
                }
                *root = left_node;
                return;
            }
        }
        if (((lexical_analyze_array[*index]).value).type == OPERATOR && 
            ((lexical_analyze_array[*index]).value).operator_ == OPERATOR_END)
        {
            struct Value new_node_value = (lexical_analyze_array[*index]).value;
            (*index)++;
            ON_DEBUG(printf("go to get_operator after operator end\n");)
            ON_DEBUG(getchar();)
            get_operator(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
            Errors_of_tree error = create_new_node(root, &(new_node_value), left_node, right_node);
            if (error != NO_ERRORS_TREE)
            {
                fprintf(stderr, "error = %d\n", error);
                abort();
            }
            return;
        }
        if (left_node != NULL)
        {
            fprintf(stderr, "Syntax Error! There is no end operator after assignment\n");
            abort();
        }
        // if (right_node == NULL)
        // {
        //     *root = left_node;
        // }
    }
    ON_DEBUG(printf("index in get_operator = %d\n", *index);)
    return;
}


static void get_number(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    ON_DEBUG(printf("index in get_number = %d\n", *index);)
    Errors_of_tree error = create_new_node(root, &((lexical_analyze_array[*index]).value), NULL, NULL);
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error = %d\n", error);
        abort();
    }
    (*index)++;
    //printf("root->value = %d\n", ((*root)->value).number);
    //printf("root = %p\n", (*root));
    return;
}

static void get_variable(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    
    if (((lexical_analyze_array[*index]).value).type == VARIABLE)
    {
        Errors_of_tree error = create_new_node(root, &((lexical_analyze_array[*index]).value), NULL, NULL);
        if (error != NO_ERRORS_TREE)
        {
            fprintf(stderr, "error = %d\n", error);
            abort();
        }
        (*index)++;
    }
    ON_DEBUG(printf("index in get_variable = %d\n", *index);)
    ON_DEBUG(getchar();)
    return;
}


static void get_expression_with_comparison_operations(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_expression_with_comparison_operations\n");)
    ON_DEBUG(getchar();)
    get_staples_expression_or_number_or_variable(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    bool is_in_while = false;
    while (((lexical_analyze_array[*index]).value).type == COMP_OPERATION)
    {
        is_in_while = true;
        struct Value new_node_value = (lexical_analyze_array[*index]).value;
        // transform_to_comparison_operation(str, &((*lexical_analyze_array[0]).value));
        (*index)++;
        ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_expression_with_comparison_operations\n");)
        ON_DEBUG(getchar();)
        get_staples_expression_or_number_or_variable(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        if (error != NO_ERRORS_TREE)
        {
            fprintf(stderr, "error = %d\n", error);
            abort();
        }
    }
    // if (!is_in_while)
    // {
    //     bool verdict = is_comparison_operation(str);
    //     if (!verdict)
    //     {
    //         *buffer = old;
    //         *index = old_index;
    //     }
    // }
    ON_DEBUG(printf("index in get_expression_with_comparison_operations = %d\n", *index);)
    return;
}

static void get_staples_expression_or_number_or_variable(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    ON_DEBUG(printf("index = %d\n", *index);)
    ON_DEBUG(printf("lexical_analyze_array[*index].type = %d\n", ((lexical_analyze_array[*index]).value).type);)
    //printf("lexical_analyze_array[*index].operator_ = %d\n", ((lexical_analyze_array[*index]).value).operator_);
    if (((lexical_analyze_array[*index]).value).type == OPERATOR &&
        ((lexical_analyze_array[*index]).value).operator_ == OPERATOR_ROUND_BRACKET_OPEN)
    {
        (*index)++;
        ON_DEBUG(printf("go to get_expression_with_plus_or_minus from get_staples_expression_or_number_or_variable\n");)
        ON_DEBUG(getchar();)
        get_expression_with_plus_or_minus(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
        // if (*buffer[0] != ')')
        // {
        //     syntax_error(buffer, index);
        // }
        if (((lexical_analyze_array[*index]).value).operator_ != OPERATOR_ROUND_BRACKET_CLOSE)
        {
            fprintf(stderr, "Syntax Error! There is no close bracket after expression\n");
            abort();
        }
        (*index)++;
        return;
    }
    else if (((lexical_analyze_array[*index]).value).type == VARIABLE)
    {
        ON_DEBUG(printf("go to get_variable from get_staples_expression_or_number_or_variable\n");)
        ON_DEBUG(getchar();)
        get_variable(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
        return;
    }
    else if (((lexical_analyze_array[*index]).value).type == NUMBER)
    {
        //printf("digit\n");
        //printf("*buffer[0] = %d\n", *buffer[0] - '0');
        ON_DEBUG(printf("go to get_number from get_staples_expression_or_number_or_variable\n");)
        ON_DEBUG(getchar();)
        get_number(root, lexical_analyze_array, len_of_lexical_analyze_array, index, parent);
        // ON_DEBUG(printf("root after get_number = %p\n", *root);)
        // ON_DEBUG(printf("root value after get_number = %f\n", ((*root)->value).number);)
        //printf("*buffer[0] = %c\n", *buffer[0]);
        //printf("root in staples = %p\n", (*root));
        return;
    }
    ON_DEBUG(printf("index in get_staples_expression_or_number_or_variable = %d\n", *index);)
    return;
}
static void get_expression_with_pow(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_expression_with_pow\n");)
    ON_DEBUG(getchar();)
    //get_expression_with_comparison_operations(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    get_staples_expression_or_number_or_variable(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    //get_staples_expression_or_number_or_variable(lexical_analyze_array, buffer, end_pointer, index);
    //ON_DEBUG(printf("left_node in pow = %f\n", (left_node->value).number);)
    //printf("left_node = %d\n", (left_node->value).number);
    while (((lexical_analyze_array[*index]).value).operation == OP_DEG)
    {
        struct Value new_node_value = (lexical_analyze_array[*index]).value;
        (*index)++;
        ON_DEBUG(printf("go to get_staples_expression_or_number_or_variable from get_expression_with_pow\n");)
        ON_DEBUG(getchar();)
        
        get_staples_expression_or_number_or_variable(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        //get_expression_with_comparison_operations(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        if (error != NO_ERRORS_TREE)
        {
            fprintf(stderr, "error = %d\n", error);
            abort();
        }
        // left_node = copy_node(*root, parent);
        //printf("root->type = %d\n", ((*root)->value).type);
    }
    if (right_node == NULL)
    {
        //ON_DEBUG(printf("left_node in pow if right_node null = %p\n", left_node);)
        // if (*root != NULL)
        // {
        //     free(*root);
        //     *root = NULL;
        // }
        *root = left_node;
        //(*root)->parent_node = parent;
        //*root = copy_node(left_node, left_node->parent_node);
    }
    //ON_DEBUG(printf("Here is root has value = %f\n", ((*root)->value).number);)
    ON_DEBUG(printf("index in get_expression_with_pow = %d\n", *index);)
    return;
}

static void get_expression_with_mul_or_div(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_expression_with_pow from get_expression_with_mul_or_div\n");)
    ON_DEBUG(getchar();)
    get_expression_with_pow(&left_node, lexical_analyze_array,len_of_lexical_analyze_array, index, parent);
    //ON_DEBUG(printf("left_node in mul or div = %p\n", left_node);)
    //printf("left_node = %d\n", (left_node->value).number);
    while (((lexical_analyze_array[*index]).value).operation == OP_MUL ||
           ((lexical_analyze_array[*index]).value).operation == OP_DIV)
    {
        //printf("root address in mul or div = %p\n", *root);
        //printf("left_node parent address in mul or div = %p\n", left_node->parent_node);
        ON_DEBUG(printf("get * or / operation\n");)
        ON_DEBUG(getchar();)
        struct Value new_node_value = (lexical_analyze_array[*index]).value;
        (*index)++;
        ON_DEBUG(printf("go to get_expression_with_pow from get_expression_with_mul_or_div\n");)
        ON_DEBUG(getchar();)
        get_expression_with_pow(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        if (error != NO_ERRORS_TREE)
        {
            fprintf(stderr, "error = %d\n", error);
            abort();
        }
        //printf("root address = %p\n", *root);
        // left_node = copy_node(*root, parent);
        //printf("root->type = %d\n", ((*root)->value).type);
    }
    if (right_node == NULL)
    {
        //ON_DEBUG(printf("left_node in mul or div if right_node null = %p\n", left_node);)
        // if (*root != NULL)
        // {
        //     free(*root);
        //     *root = NULL;
        // }
        *root = left_node;
        //(*root)->parent_node = parent;
        //*root = copy_node(left_node, NULL);
    }
    ON_DEBUG(printf("index in get_expression_with_mul_or_div = %d\n", *index);)
    return;
}

static void get_expression_with_plus_or_minus(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index, struct Node *parent)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    struct Node *left_node = NULL;
    struct Node *right_node = NULL;
    ON_DEBUG(printf("go to get_expression_with_mul_or_div from get_expression_with_plus_or_minus\n");)
    ON_DEBUG(getchar();)
    get_expression_with_mul_or_div(&left_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
    //ON_DEBUG(printf("left_node in plus or minus = %f\n", (left_node->value).number);)
    if (left_node == NULL)
    {
        struct Node new_left_node = {.value = {.type = NUMBER, .number = 0}};
        int id = 0;
        get_number(&left_node, &new_left_node, len_of_lexical_analyze_array, &id, *root);
    }
    while (((lexical_analyze_array[*index]).value).operation == OP_ADD ||
           ((lexical_analyze_array[*index]).value).operation == OP_SUB)
    {
        ON_DEBUG(printf("get + or - operation\n");)
        ON_DEBUG(getchar();)
        struct Value new_node_value = (lexical_analyze_array[*index]).value;
        (*index)++;
        //printf("index = %d\n", *index);
        ON_DEBUG(printf("go to get_expression_with_mul_or_div from get_expression_with_plus_or_minus\n");)
        ON_DEBUG(getchar();)
        get_expression_with_mul_or_div(&right_node, lexical_analyze_array, len_of_lexical_analyze_array, index, *root);
        Errors_of_tree error = create_new_node(root, &new_node_value, left_node, right_node);
        if (error != NO_ERRORS_TREE)
        {
            fprintf(stderr, "error = %d\n", error);
            abort();
        }
        // left_node = copy_node(*root, NULL);
        // ON_DEBUG(printf("root address = %p\n", *root);)
    }
    if (right_node == NULL)
    {
        //ON_DEBUG(printf("left_node in plus or minus if right_node null = %p\n", left_node);)
        // if (*root != NULL)
        // {
        //     free(*root);
        //     *root = NULL;
        // }
        *root = left_node;
        //(*root)->parent_node = parent;
        //*root = copy_node(left_node, NULL);
    }
    ON_DEBUG(printf("index in get_expression_with_plus_or_minus = %d\n", *index);)
    return;
}


static void parse_information_from_lexical_analyze_array_by_recursive_descent(struct Node **root, struct Node *lexical_analyze_array, int len_of_lexical_analyze_array, int *index)
{
    if (*index >= len_of_lexical_analyze_array)
    {
        return;
    }
    ON_DEBUG(printf("go to get_operator from parse_information_from_lexical_analyze_array_by_recursive_descent\n");)
    ON_DEBUG(getchar();)
    get_operator(root, lexical_analyze_array, len_of_lexical_analyze_array, index, NULL);
    //get_expression_with_plus_or_minus(lexical_analyze_array, buffer, end_pointer, index);
    //printf("general root = %p\n", *root);
    //printf("*buffer[0] = %c\n", *buffer[0]);
    // if (*buffer[0] != '$')
    // {
    //     //printf("Here!\n");
    //     syntax_error(buffer, index);
    // }
    return;
}

static void parse_information_from_file_to_lexical_analyze_array(struct Node **lexical_analyze_array, char **buffer, char *end_pointer, int *len_of_lexical_analyze_array)
{
    if (*buffer == end_pointer)
    {
        return;
    }
    Value_type type = FUNCTION;
    char str_before_function[100] = {};
    while (*buffer != end_pointer && *buffer[0] != '$')
    {
        *buffer = skip_spaces(*buffer, end_pointer);
        if (*buffer[0] == '$')
        {
            break;
        }
        ON_DEBUG(printf("*buffer[0] = %c\n", *buffer[0]);)
        if (*buffer[0] == ';')
        {
            ON_DEBUG(printf("it is operator ;\n");)
            ON_DEBUG(getchar();)
            ((*lexical_analyze_array[0]).value).type = OPERATOR;
            ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_END;
            (*lexical_analyze_array)++;
            (*buffer)++;
            (*len_of_lexical_analyze_array)++;
        }
        else if (isalpha(*buffer[0]))
        {
            ON_DEBUG(printf("it is operator if or while or variable or function\n");)
            ON_DEBUG(getchar();)
            char str[100] = {};
            *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
            ON_DEBUG(printf("str = %s\n", str);)
            
            char old_str[100] = {};
            strncpy(old_str, str, strlen(str));
            bool verdict = parse_operator_to_lexical_analyze_array(lexical_analyze_array, str, len_of_lexical_analyze_array);
            if (verdict)
            {
                strncpy(str_before_function, old_str, strlen(old_str));
                ON_DEBUG(printf("str before function = %s\n", str_before_function);)
                continue;
            }
            //strncpy(str_before_function, old_str, strlen(old_str));
            //ON_DEBUG(printf("str before function = %s\n", str_before_function);)
            char *old_buffer = *buffer;
            *buffer = get_operation_from_file(str, 100, *buffer, end_pointer);
            *buffer = old_buffer;
            if (strcasecmp(str, "(") == 0)
            {
                if (strcasecmp(str_before_function, "def") == 0)
                {
                    type = FUNCTION;
                }
                else
                {
                    type = CALLER_OF_FUNCTION;
                }
                ON_DEBUG(printf("it is a function\n");)
                ON_DEBUG(getchar();)
                parse_function_to_lexical_analyze_array(lexical_analyze_array, old_str, len_of_lexical_analyze_array, type);
                continue;
            }
            ON_DEBUG(printf("it is a variable\n");)
            ON_DEBUG(getchar();)
            parse_variable_to_lexical_analyze_array(lexical_analyze_array, old_str, len_of_lexical_analyze_array);
            continue;
        }
        else if (isdigit(*buffer[0]))
        {
            ON_DEBUG(printf("it is a number\n");)
            ON_DEBUG(getchar();)
            char str[100] = {};
            *buffer = get_value_from_file(str, 100, *buffer, end_pointer);
            parse_number_to_lexical_analyze_array(lexical_analyze_array, str, len_of_lexical_analyze_array);
        }
        else
        {
            char str[100] = {};
            *buffer = get_operation_from_file(str, 100, *buffer, end_pointer);
            ON_DEBUG(printf("operation = %s\n", str);)
            bool verdict = parse_operator_to_lexical_analyze_array(lexical_analyze_array, str, len_of_lexical_analyze_array);
            if (verdict)
            {
                char s[100] = {};
                strncpy(str_before_function, s, 100);
                strncpy(str_before_function, str, strlen(str));
                ON_DEBUG(printf("str before function = %s\n", str_before_function);)
                continue;
            }
            ON_DEBUG(printf("it is an operation\n");)
            ON_DEBUG(getchar();)
            verdict = parse_comp_operation_to_lexical_analyze_array(lexical_analyze_array, str, len_of_lexical_analyze_array);
            if (!verdict)
            {
                fprintf(stderr, "ERROR! UNKNOWN TYPE OF THE TOKEN\n");
                abort();
            }
        }

    }
    return;
}


static bool parse_operator_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array)
{
    if (lexical_analyze_array == NULL || str == NULL)
    {
        fprintf(stderr, "ERROR OF PARSING OPERATOR\n");
        abort();
    }
    ((*lexical_analyze_array[0]).value).type = OPERATOR;
    if (strcasecmp(str, "if") == 0)
    {
        ON_DEBUG(printf("it is operator if\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_IF;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    if (strcasecmp(str, "while") == 0)
    {
        ON_DEBUG(printf("it is operator while\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_WHILE;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    if (strcasecmp(str, "else") == 0)
    {
        ON_DEBUG(printf("it is operator else\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_ELSE;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    if (strcasecmp(str, "def") == 0)
    {
        ON_DEBUG(printf("it is operator def\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_DEF;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    if (strcasecmp(str, "return") == 0)
    {
        ON_DEBUG(printf("it is operator return\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_RETURN;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    if (strcasecmp(str, "=") == 0)
    {
        ON_DEBUG(printf("it is operator assignment\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_ASSIGNMENT;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    if (strcasecmp(str, "(") == 0)
    {
        ON_DEBUG(printf("it is operator round bracket open\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_ROUND_BRACKET_OPEN;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, ")") == 0)
    {
        ON_DEBUG(printf("it is operator round bracket close\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_ROUND_BRACKET_CLOSE;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "{") == 0)
    {
        ON_DEBUG(printf("it is operator curly bracket open\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_CURLY_BRACKET_OPEN;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "}") == 0)
    {
        ON_DEBUG(printf("it is operator curly bracket close\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_CURLY_BRACKET_CLOSE;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "[") == 0)
    {
        ON_DEBUG(printf("it is operator square bracket open\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_SQUARE_BRACKET_OPEN;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "]") == 0)
    {
        ON_DEBUG(printf("it is operator square bracket close\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_SQUARE_BRACKET_CLOSE;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, ",") == 0)
    {
        ON_DEBUG(printf("it is operator comma\n");)
        ON_DEBUG(getchar();)
        ((*lexical_analyze_array[0]).value).operator_ = OPERATOR_COMMA;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    return false;
}

static void parse_variable_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array)
{
    if (lexical_analyze_array == NULL || str == NULL)
    {
        fprintf(stderr, "ERROR OF PARSING VARIABLE\n");
        abort();
    }

    ((*lexical_analyze_array[0]).value).type = VARIABLE;
    size_t len_of_str = strlen(str);
    strncpy(((*lexical_analyze_array[0]).value).variable_name, str, len_of_str);
    (*lexical_analyze_array)++;
    (*len_of_lexical_analyze_array)++;
    return;
}

static void parse_number_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array)
{
    if (lexical_analyze_array == NULL || str == NULL)
    {
        fprintf(stderr, "ERROR OF PARSING NUMBER\n");
        abort();
    }
    char *end = NULL;
    double value = strtod(str, &end);
    ((*lexical_analyze_array[0]).value).type = NUMBER;
    ((*lexical_analyze_array[0]).value).number = value;
    (*lexical_analyze_array)++;
    (*len_of_lexical_analyze_array)++;
    return;
}

static void parse_function_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array, Value_type type)
{
    if (lexical_analyze_array == NULL || str == NULL)
    {
        fprintf(stderr, "ERROR OF PARSING FUNCTION\n");
        abort();
    }
    bool flag = false;
    for (size_t index = 0; index < size_of_built_in_functions; index++)
    {
        if (strcasecmp(built_in_functions[index], str) == 0)
        {
            ((*lexical_analyze_array[0]).value).type = BUILT_IN_FUNCTION;
            flag = true;
            break;
        }
    }
    if (!flag)
    {
        ((*lexical_analyze_array[0]).value).type = type;
    }
    size_t len_of_str = strlen(str);
    strncpy(((*lexical_analyze_array[0]).value).function_name, str, len_of_str);
    (*lexical_analyze_array)++;
    (*len_of_lexical_analyze_array)++;
    return;
}

static bool parse_comp_operation_to_lexical_analyze_array(struct Node **lexical_analyze_array, char *str, int *len_of_lexical_analyze_array)
{
    if (lexical_analyze_array == NULL || str == NULL)
    {
        fprintf(stderr, "ERROR OF PARSING COMPARISON OPERATION\n");
        abort();
    }
    ((*lexical_analyze_array[0]).value).type = COMP_OPERATION;
    if (strcasecmp(str, ">=") == 0)
    {
        ((*lexical_analyze_array[0]).value).comp_operation = OP_MORE_OR_EQUAL;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "<=") == 0)
    {
        ((*lexical_analyze_array[0]).value).comp_operation = OP_LESS_OR_EQUAL;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, ">") == 0)
    {
        ((*lexical_analyze_array[0]).value).comp_operation = OP_MORE;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "<") == 0)
    {
        ((*lexical_analyze_array[0]).value).comp_operation = OP_LESS;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "!=") == 0)
    {
        ((*lexical_analyze_array[0]).value).comp_operation = OP_NOT_EQUAL;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else if (strcasecmp(str, "==") == 0)
    {
        ((*lexical_analyze_array[0]).value).comp_operation = OP_EQUAL;
        (*lexical_analyze_array)++;
        (*len_of_lexical_analyze_array)++;
        return true;
    }
    else
    {
        ((*lexical_analyze_array[0]).value).type = OPERATION;
        if (str[0] == '+')
        {
            ((*lexical_analyze_array[0]).value).operation = OP_ADD;
            (*lexical_analyze_array)++;
            (*len_of_lexical_analyze_array)++;
            return true;
        }
        else if (str[0] == '-')
        {
            ((*lexical_analyze_array[0]).value).operation = OP_SUB;
            (*lexical_analyze_array)++;
            (*len_of_lexical_analyze_array)++;
            return true;
        }
        else if (str[0] == '*')
        {
            ((*lexical_analyze_array[0]).value).operation = OP_MUL;
            (*lexical_analyze_array)++;
            (*len_of_lexical_analyze_array)++;
            return true;
        }
        else if (str[0] == '/')
        {
            ((*lexical_analyze_array[0]).value).operation = OP_DIV;
            (*lexical_analyze_array)++;
            (*len_of_lexical_analyze_array)++;
            return true;
        }
        else if (str[0] == '^')
        {
            ((*lexical_analyze_array[0]).value).operation = OP_DEG;
            (*lexical_analyze_array)++;
            (*len_of_lexical_analyze_array)++;
            return true;
        }
    }
    return false;
}