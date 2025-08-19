#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "assembler_structures.h"
#include "../../Reader/include/tree.h"
#include "stack.h"
static void bypass_of_tree(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements);// struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void choose_way_of_operating(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements); // struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void process_operator_assignment(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements);// struct Label **all_variables);
static void process_built_in_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements); //struct Label **all_variables);
static void process_caller_of_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements);
static void add_parametres_for_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, size_t *counter_of_parametres, Value_type type);
static void process_expression_after_assignment(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, size_t *counter_of_parametres, Value_type type);
static void process_global_variable(struct Value *value, FILE *file_pointer, struct Special_elements_for_processing *elements);
static void process_local_variable(struct Value *value, FILE *file_pointer, struct Special_elements_for_processing *elements);//struct Label **all_variables);
static void process_operation(struct Value *value, FILE *file_pointer);
static void process_operator_if(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements);//struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void process_operator_else(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements); //struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void process_comparison_expression(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, Programm_operators operator_);//struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, Programm_operators operator_);
static void process_expression_after_comparison(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, Value_type type);//struct Label **all_variables, struct Labels **all_labels);
static void process_comparison_operation(struct Value *value, FILE *file_pointer);
static void process_operator_while(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements); //struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while);
static void process_definition_of_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements);
static void process_operator_return(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements);
static void get_local_memory(struct Value *value, struct Special_elements_for_processing *elements);
static void create_deg_operation(FILE *file_pointer);
static void create_function_print(FILE *file_pointer);
static void create_function_strlen(FILE *file_pointer);
static void transform_number(FILE *file_pointer, struct Value *value);

static void transform_number(FILE *file_pointer, struct Value *value)
{
    if (value == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "Error!\n");
        abort();
    }
    if (value->type != NUMBER)
    {
        return;
    }
    char buffer[50] = {};
    if (value->type_of_number == INT)
    {
        snprintf(buffer, 50, "%d", (int)(value->number));
    }
    else
    {
        snprintf(buffer, 50, "%lf", value->number);
    }
    fprintf(file_pointer, "\tmov rbx, \'%s\'\n\tpush rbx\n", buffer);
    return;
}


Errors_of_ASM transform_programm_to_assembler(struct Tree *tree, struct Labels **all_labels)
{
    if (tree == NULL)
    {
        return ERROR_OF_OPERATING_TREE;
    }
    FILE *file_pointer = fopen("source/nasm/asm_programm.asm", "w");
    if (file_pointer == NULL)
    {
        return ERROR_OF_CREATE_ASM_FILE;
    }
    fprintf(file_pointer, "%%include \"include/asm_built_in_functions.inc\"\n\n");
    fprintf(file_pointer, "section .data\n\tbuffer_for_text db 256 dup(0)\n\tlen_of_buffer equ $ - buffer_for_text\n"
                          "\tbuffer_index dq 0\n\tbuffer_for_input db 256 dup(0)\n\tbuffer_size equ $ - buffer_for_input\n");
    fprintf(file_pointer, "section .text\n\tglobal _start\n\textern pow\n");
    //create_function_strlen(file_pointer);
    //create_function_print(file_pointer);

    struct Special_elements_for_processing elements = {0};
    elements.all_labels = *all_labels;
    elements.all_variables = (struct Label *) calloc (SIZE_OF_ALL_VARIABLES, sizeof(struct Label));
    if (elements.all_variables == NULL)
    {
        return ERROR_OF_OPERATING_TREE;
    }
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        ((elements.all_variables)[index]).address = -1;
    }
    elements.all_functions = (struct Function_type *) calloc(SIZE_OF_ALL_VARIABLES, sizeof(struct Function_type));
    if (elements.all_functions == NULL)
    {
        return ERROR_OF_OPERATING_TREE;
    }
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        ((elements.all_functions)[index]).all_local_variables = (struct Label *) calloc (SIZE_OF_ALL_VARIABLES, sizeof(struct Label));
        if (((elements.all_functions)[index]).all_local_variables == NULL)
        {
            return ERROR_OF_OPERATING_TREE;
        }
    }
    elements.all_registers = (struct Register *) calloc (SIZE_OF_ALL_REGISTERS, sizeof(struct Register));
    if (elements.all_registers == NULL)
    {
        return ERROR_OF_OPERATING_TREE;
    }
    for (size_t index = 0; index < SIZE_OF_ALL_REGISTERS; index++)
    {
        ((elements.all_registers)[index]).is_free = true;
    }
    // struct MyStack stack_if = {0};
    // struct MyStack stack_while = {0};
    Errors error = NO_ERRORS;
    error = STACK_CTOR(&(elements.stack_if), 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = STACK_CTOR(&(elements.stack_while), 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = STACK_CTOR(&(elements.stack_else), 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = STACK_CTOR(&(elements.current_function), 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = STACK_CTOR(&(elements.stack_for_operations), 10);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    elements.counter_of_if = 0;
    elements.counter_of_while = 0;
    elements.counter_of_else = 0;
    elements.start_local_memory_address = 250;
    elements.is_body_of_functions = false;
    elements.is_assignment = false;
    if (!tree->are_any_functions)
    {
        char start_str[50] = "_start:";
        fprintf(file_pointer, "%s\n", start_str);
        fprintf(file_pointer, "\tpush rbp\n\tmov rbp, rsp\n");
        fprintf(file_pointer, "\tadd rsp, -16\n");
    }
    bypass_of_tree(tree->root, file_pointer, &elements);// &all_variables, all_labels, &counter_of_if, &counter_of_while, &stack_if, &stack_while);
    *all_labels = elements.all_labels;
    fprintf(file_pointer, "\tpop rbp\n\tmov rax, 60\n\txor rdi, rdi\n\tsyscall\n");
    free(elements.all_variables);
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        free(((elements.all_functions)[index]).all_local_variables);
    }
    free(elements.all_functions);
    free(elements.all_registers);
    fclose(file_pointer);
    error = stack_destructor(&(elements.stack_if));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = stack_destructor(&(elements.stack_while));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = stack_destructor(&(elements.stack_else));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = stack_destructor(&(elements.current_function));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    error = stack_destructor(&(elements.stack_for_operations));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    return NO_ASM_ERRORS;
}

static void create_function_strlen(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        fprintf(stderr, "Error! Output file is not existed\n");
        abort();
    }
    fprintf(file_pointer, "strlen:\n");
    fprintf(file_pointer, "\tmov rcx, -1\n\txor al, al\n\tcld\n\trepne scasb\n\tnot rcx\n\tdec rcx\n\tmov rax, rcx\n\tret\n");
    return;
}

static void create_function_print(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        fprintf(stderr, "Error! Output file is not existed\n");
        abort();
    }
    fprintf(file_pointer, "\n\nprint:\n");
    fprintf(file_pointer, "\tpush rbp\n\tmov rbp, rsp\n");
    fprintf(file_pointer, "\tcmp rdi, 0\n\tje print_enter\n");
    fprintf(file_pointer, "\tmov r10, rdi\n\txor r11, r11\n");
    fprintf(file_pointer, "\tlea rbx, [rbp + 8 * (rdi + 1)]\n");
    fprintf(file_pointer, "cycle_processing:\n");
    fprintf(file_pointer, "\tcmp r10, 0\n\tje print_enter\n");
    fprintf(file_pointer, "\tmov rdi, rbx\n\tcall strlen\n\tmov r12, rax\n\tprint_str rbx, r12\n\tpush 10\n\tprint_str rsp, 1\n\tadd rsp, 1\n\tsub rbx, 8\n\tdec r10\n\tjmp cycle_processing\n");
    fprintf(file_pointer, "print_enter:\n\tpush 10\n\tprint_str rsp, 1\n\tadd rsp, 1\n");
    fprintf(file_pointer, "\tmov rsp, rbp\n\tpop rbp\n");
    fprintf(file_pointer, "\tret\n");
}

//static void bypass_of_tree(struct Node *root, FILE *file_pointer, struct Label **all_variables, struct Labels **all_labels, size_t *counter_of_if, size_t *counter_of_while, struct MyStack *stack_if, struct MyStack *stack_while)
static void bypass_of_tree(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF WRITING TO FILE\n");
        abort();
    }
    bypass_of_tree(root->left, file_pointer, elements);// all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    choose_way_of_operating(root, file_pointer, elements); //all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    if ((root->value).type != OPERATOR ||
        ((root->value).operator_ != OPERATOR_IF &&
         (root->value).operator_ != OPERATOR_WHILE &&
         (root->value).operator_ != OPERATOR_DEF))
    {
        bypass_of_tree(root->right, file_pointer, elements);// all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    return;
}

static void choose_way_of_operating(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF OPERATING TREE\n");
        abort();
    }
    // if ((root->value).type != OPERATOR)
    // {
    //     return;
    // }
    if ((root->value).type == OPERATOR)
    {
        switch ((root->value).operator_)
        {
            case OPERATOR_ASSIGNMENT:
            {
                process_operator_assignment(root, file_pointer, elements);// all_variables);
                return;
            }
            case OPERATOR_IF:
            {
                process_operator_if(root, file_pointer, elements);// all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
                return;
            }
            case OPERATOR_WHILE:
            {
                process_operator_while(root, file_pointer, elements);//all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
                return;
            }
            case OPERATOR_DEF:
            {
                process_definition_of_function(root, file_pointer, elements);
                return;
            }
            case OPERATOR_RETURN:
            {
                process_operator_return(root, file_pointer, elements);
                return;

            }
            default: return;
        }
    }
    else if ((root->value).type == BUILT_IN_FUNCTION)
    {
        //printf("call process_built_in_function from choose_way_of_operating with function = %s\n", (root->value).function_name);
        //getchar();
        process_built_in_function(root, file_pointer, elements); //all_variables);
    }
    else if ((root->value).type == CALLER_OF_FUNCTION)
    {
        
        process_caller_of_function(root, file_pointer, elements);
        
    }
    else 
    {
        return;
    }
    return;
}

static void process_operator_while(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR WHILE\n");
        abort();
    }
    (elements->counter_of_while)++;
    elements->is_assignment = false;
    Errors error = stack_push(&(elements->stack_while), (double)(elements->counter_of_while));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    process_comparison_expression(root->left, file_pointer, elements, OPERATOR_WHILE); //all_variables, all_labels, counter_of_if, counter_of_while, OPERATOR_WHILE);
    fprintf(file_pointer, "begin_while%lu:\n", elements->counter_of_while);
    bypass_of_tree(root->right, file_pointer, elements); //all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    if ((elements->stack_while).size == 0)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, elements); //all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    while ((elements->stack_while).size != 0)
    {
        Stack_Elem_t element = 0;
        error = stack_pop(&(elements->stack_while), &element);
        if (error != NO_ERRORS)
        {
            fprintf(stderr, "error of stack = %d\n", error);
            abort();
        }
        elements->counter_of_while = (size_t)element;
        process_comparison_expression(root->left, file_pointer, elements, OPERATOR_WHILE); //all_variables, all_labels, counter_of_if, &counter, OPERATOR_WHILE);
        fprintf(file_pointer, "end_while%d:\n", (int)element);
        size_t index = 0;
        while (strlen(((elements->all_labels)[index]).name) != 0 && index < SIZE_OF_ALL_VARIABLES)
        {
            index++;
        }
        if (index < SIZE_OF_ALL_VARIABLES)
        {
            char str[50] = "end_while";
            const size_t current_len = strlen(str);
            snprintf(str + current_len, sizeof(str) - current_len, "%d:", (int)element);
            strncpy(((elements->all_labels)[index]).name, str, strlen(str));
        }
        if (element > 1)
        {
            bypass_of_tree(root->node_after_operator, file_pointer, elements); //all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
        }
    }
    if (elements->counter_of_while == 1)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, elements);//all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    //bypass_of_tree(root->right, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    return;
}

static void process_operator_if(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR IF\n");
        abort();
    }
    (elements->counter_of_if)++;
    elements->is_assignment = false;
    Errors error = stack_push(&(elements->stack_if), (double)(elements->counter_of_if));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    process_comparison_expression(root->left, file_pointer, elements, OPERATOR_IF); //all_variables, all_labels, counter_of_if, counter_of_while, OPERATOR_IF);
    fprintf(file_pointer, "begin_if%lu:\n", elements->counter_of_if);
    bypass_of_tree(root->right, file_pointer, elements);//all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    process_operator_else(root->node_for_operator_else, file_pointer, elements); //all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    if ((elements->stack_if).size == 0)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, elements);//all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    if ((elements->stack_if).size != 0)
    {
        Stack_Elem_t element = 0;
        error = stack_pop(&(elements->stack_if), &element);
        if (error != NO_ERRORS)
        {
            fprintf(stderr, "error of stack = %d\n", error);
            abort();
        }
        fprintf(file_pointer, "end_if%d:\n", (int)element);
        size_t index = 0;
        while (strlen(((elements->all_labels)[index]).name) != 0 && index < SIZE_OF_ALL_VARIABLES)
        {
            index++;
        }
        if (index < SIZE_OF_ALL_VARIABLES)
        {
            char str[50] = "end_if";
            const size_t current_len = strlen(str);
            snprintf(str + current_len, sizeof(str) - current_len, "%d:", (int)element);
            strncpy(((elements->all_labels)[index]).name, str, strlen(str));
        }
        if (element > 1)
        {
            bypass_of_tree(root->node_after_operator, file_pointer, elements);//all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
        }
    }
    if (elements->counter_of_if == 1)
    {
        bypass_of_tree(root->node_after_operator, file_pointer, elements); //all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    }
    //bypass_of_tree(root->node_after_operator, file_pointer, all_variables, all_labels, counter_of_if, counter_of_while, stack_if, stack_while);
    return;
}

static void process_operator_else(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL)
    {
        return;
    }
    (elements->counter_of_else)++;
    elements->is_assignment = false;
    Errors error = stack_push(&(elements->stack_else), (elements->counter_of_else));
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    if (file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR ELSE\n");
        abort();
    }
    if (elements->counter_of_if == 0)
    {
        fprintf(stderr, "ERROR! There is no operator if before else\n");
        abort();
    }
    Stack_Elem_t element = 0;
    error = stack_element(&(elements->stack_if), &element);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    fprintf(file_pointer, "\tjmp end_if%lu\n", (size_t)element);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    char str_begin[50] = "begin_else";
    char str_end[50] = "end_else";
    const size_t current_len_begin = strlen(str_begin);
    const size_t current_len_end = strlen(str_end);
    snprintf(str_begin + current_len_begin, sizeof(str_begin) - current_len_begin, "%lu:", (size_t)element);
    snprintf(str_end + current_len_end, sizeof(str_end) - current_len_end, "%lu:", (size_t)element);
    size_t index = 0;
    while (index < SIZE_OF_ALL_VARIABLES && strlen(((elements->all_labels)[index]).name) != 0)
    {
        //printf("index = %lu\n", index);
        if (strcasecmp(str_begin, ((elements->all_labels)[index]).name) == 0)
        {
            index = SIZE_OF_ALL_VARIABLES;
        }
        index++;
    }
    if (index < SIZE_OF_ALL_VARIABLES)
    {
        strncpy(((elements->all_labels)[index]).name, str_begin, strlen(str_begin));
        index++;
    }
    fprintf(file_pointer, "%s\n", str_begin);
    bypass_of_tree(root->right, file_pointer, elements);
    if ((elements->stack_else).size != 0)
    {
        fprintf(file_pointer, "end_else%lu:\n", (size_t)element);
        while (index < SIZE_OF_ALL_VARIABLES && strlen(((elements->all_labels)[index]).name) != 0)
        {
            index++;
        }
        if (index < SIZE_OF_ALL_VARIABLES)
        {
            snprintf(str_end + current_len_end, sizeof(str_end) - current_len_end, "%lu:", (size_t)element);
            strncpy(((elements->all_labels)[index]).name, str_end, strlen(str_end));
        }
    }
    return;

}

static void get_local_memory(struct Value *value, struct Special_elements_for_processing *elements)
{
    if (value == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF GETTING LOCAL MEMORY\n");
        abort();
    }
    bool flag = false;
    bool is_overflow = true;
    size_t id = 0;
    for (id = 0; id < SIZE_OF_ALL_VARIABLES; id++)
    {
        //printf("function name = %s\n", (elements->all_functions)[id].function_name);
        if (strcasecmp((elements->all_functions)[id].function_name, value->function_name) == 0)
        {
            flag = true;
            is_overflow = false;
            break;
        }
        else if (strlen((elements->all_functions)[id].function_name) == 0)
        {
            strncpy((elements->all_functions)[id].function_name, value->function_name, strlen(value->function_name));
            //printf("start_local_memory_address = %lu\n", elements->start_local_memory_address);
            is_overflow = false;
            (elements->all_functions)[id].last_free_address = -4;
            flag = true;
            break;
        }
    }
    //printf("\n\n");
    if (!flag)
    {
        fprintf(stderr, "Error! There is not enough memory in array all functions\n");
        abort();
    }
    if (is_overflow)
    {
        fprintf(stderr, "Error! This is overflow of ram memory\n");
        abort();
    }
    Errors error = stack_push(&(elements->current_function), (Stack_Elem_t)id);
    //printf("elements->current_function = %lf\n", (Stack_Elem_t)id);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    return;
}

static void process_built_in_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING BUILT IN FUNCTION\n");
        abort();
    }
    if (!root->is_processed)
    {
        root->is_processed = true;
    }
    else
    {
        return;
    }
    //get_local_memory(&(root->value), elements);
    elements->is_assignment = false;
    if (strcasecmp((root->value).function_name, "input") == 0)
    {
        fprintf(file_pointer, "\tcall input\n");
        fprintf(file_pointer, "\tpush rax\n");
        return;
    }
    size_t counter_of_parametres = 0;
    //process_expression_after_assignment(root->left, file_pointer, elements, &counter_of_parametres, BUILT_IN_FUNCTION); //all_variables, &counter_of_parametres);
    add_parametres_for_function(root->left, file_pointer, elements, &counter_of_parametres, BUILT_IN_FUNCTION); //all_variables, &counter_of_parametres);
    add_parametres_for_function(root->right, file_pointer, elements, &counter_of_parametres, BUILT_IN_FUNCTION); //all_variables, &counter_of_parametres);
    if (strcasecmp((root->value).function_name, "print") == 0)
    {
        fprintf(file_pointer, "\tmov rdi, %lu\n", counter_of_parametres);
        fprintf(file_pointer, "\tcall print\n");
    }
    return;
}
static void process_definition_of_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING DEFINITION OF FUNCTION\n");
        abort();
    }
    if (!root->is_processed)
    {
        root->is_processed = true;
    }
    else
    {
        return;
    }
    elements->is_body_of_functions = true;
    elements->is_assignment = false;
    struct Node *function = root->left;
    get_local_memory(&(function->value), elements);
    fprintf(file_pointer, "%s:\n", (function->value).function_name);
    fprintf(file_pointer, "\tpush rbp\n\tmov rbp, rsp\n");
    Stack_Elem_t element = 0;
    Errors error = stack_element(&(elements->current_function), &element);
    size_t id = (size_t)element;
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    size_t counter_of_parametres = 0;
    add_parametres_for_function(function->left, file_pointer, elements, &counter_of_parametres, FUNCTION);
    add_parametres_for_function(function->right, file_pointer, elements, &counter_of_parametres, FUNCTION);
    (elements->all_functions)[id].is_parametres_processed = true;
    //printf("start_address = %lu\nend_address = %lu\n", (elements->all_functions)[id].start_local_memory_address, (elements->all_functions)[id].end_local_memory_address);
    fprintf(file_pointer, "\tsub rsp, %d\n", 8 * (counter_of_parametres * 3));
    if (counter_of_parametres != 0)
    {
        int parametre = 8 * (counter_of_parametres + 1);
        for (size_t index = 0; index < counter_of_parametres; index++, parametre -= 8)
        {
            fprintf(file_pointer, "\tmov rax, [rbp + %d]\n", parametre);
            fprintf(file_pointer, "\tmov DWORD [rbp %d], eax\n", ((elements->all_functions)[id].all_local_variables)[index].address);
        }
    }
    bypass_of_tree(root->right, file_pointer, elements);
    elements->is_body_of_functions = false;
    error = stack_pop(&(elements->current_function), &element);
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    if (root->is_last_function)
    {
        char start_str[50] = "_start:";
        fprintf(file_pointer, "%s\n", start_str);
        fprintf(file_pointer, "\tpush rbp\n\tmov rbp, rsp\n");
        fprintf(file_pointer, "\tadd rsp, -16\n");
    }
    bypass_of_tree(root->node_after_operator, file_pointer, elements);
    return;
}

static void process_operator_return(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR RETURN\n");
        abort();
    }
    size_t counter_of_parametres = 0;
    process_expression_after_assignment(root->left, file_pointer, elements, &counter_of_parametres, OPERATOR);
    fprintf(file_pointer, "\tpop rax\n\tmov rsp, rbp\n\tpop rbp\n\tret\n");
    return;
}
static void process_caller_of_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING CALLER OF FUNCTION\n");
        abort();
    }
    if (!root->is_processed)
    {
        root->is_processed = true;
    }
    else
    {
        return;
    }
    if (!elements->is_assignment)
    {
        size_t counter_of_parametres = 0;
        add_parametres_for_function(root->left, file_pointer, elements, &counter_of_parametres, CALLER_OF_FUNCTION); //all_variables, &counter_of_parametres);
        add_parametres_for_function(root->right, file_pointer, elements, &counter_of_parametres, CALLER_OF_FUNCTION); //all_variables, &counter_of_parametres);
    }
    bool is_exists = false;
    size_t index = 0;
    for (index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        if (strcasecmp((elements->all_functions)[index].function_name, (root->value).function_name) == 0)
        {
            is_exists = true;
            break;
        }
    }
    if (!is_exists)
    {
        fprintf(stderr, "Error! Definition of function %s is not exists\n", (root->value).function_name);
        abort();
    }
    fprintf(file_pointer, "\tcall %s\n\tpush rax\n", (root->value).function_name);
    // fprintf(file_pointer, "jmp %s:\n", (root->value).function_name);
    // fprintf(file_pointer, "caller_%s:\n", (root->value).function_name);
    return;
}


static void add_parametres_for_function(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, size_t *counter_of_parametres, Value_type type)
{
    if (root == NULL)
    {
        return;
    }
    //(*counter_of_parametres)++;
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }
    add_parametres_for_function(root->left, file_pointer, elements, counter_of_parametres, type); //all_variables, counter_of_parametres);
    add_parametres_for_function(root->right, file_pointer, elements, counter_of_parametres, type); //all_variables, counter_of_parametres);
    
    switch ((root->value).type)
    {
        case NUMBER:
        {
            fprintf(file_pointer, "\tpush %d\n", (int)((root->value).number));
            //transform_number(file_pointer, &(root->value));
            (*counter_of_parametres)++;
            return;
        }
        case VARIABLE:
        {
            if (!elements->is_body_of_functions)
            {
                process_global_variable(&(root->value), file_pointer, elements);
            }
            else
            {
                process_local_variable(&(root->value), file_pointer, elements);
            }
            (*counter_of_parametres)++;
            return;
        }
        case OPERATION:
        {
            process_operation(&(root->value), file_pointer);
            (*counter_of_parametres)--;
            return;
        }
        case OPERATOR:
        {
            if ((root->value).operator_ == OPERATOR_COMMA)
            {
                //add_parametres_for_function(root->right, file_pointer, all_variables);
                //process_built_in_function(root, file_pointer, all_variables);
                return;
            }
            break;
        }
        default:
        {
            fprintf(stderr, "ERROR OF UNKNOWN TYPE\n");
            abort();
        }
    }
    return;
}


static void process_operator_assignment(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (root == NULL || file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATOR ASSIGNMENT\n");
        abort();
    }
    Stack_Elem_t element = 0;
    Errors error = stack_element(&(elements->current_function), &element);
    size_t id = (size_t)element;
    elements->is_assignment = true;
    int ram_address = -1;
    bool is_exits = false;
    int empty_index = 0;
    int last_address = 0;
    if (!elements->is_body_of_functions)
    {
        for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
        {
            
            if (strcasecmp(((root->left)->value).variable_name, ((elements->all_variables)[index]).name) == 0)
            {
                ram_address = ((elements->all_variables)[index]).address;
                is_exits = true;
                break;
            }
            if (((elements->all_variables)[index]).address == -1)
            {
                empty_index = index;
                break;
            }
            last_address = ((elements->all_variables)[index]).address;
        }
    }
    else
    {
        for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
        {
            if (strcasecmp(((root->left)->value).variable_name, ((elements->all_functions)[id]).all_local_variables[index].name) == 0)
            {
                ram_address = (((elements->all_functions)[id]).all_local_variables[index]).address;
                is_exits = true;
                break;
            }
            else
            {
                if (strlen(((elements->all_functions)[id]).all_local_variables[index].name) == 0)
                {
                    is_exits = true;
                    strncpy((((elements->all_functions)[id]).all_local_variables)[index].name, ((root->left)->value).variable_name, strlen(((root->left)->value).variable_name));
                    (((elements->all_functions)[id]).all_local_variables)[index].address = ((elements->all_functions)[id]).last_free_address;
                    ((elements->all_functions)[id]).last_free_address -= 4;
                    ram_address = (((elements->all_functions)[id]).all_local_variables)[index].address;
                    break;
                }
            }
            
        }
    }
    size_t counter_of_parametres = 0;
    process_expression_after_assignment(root->right, file_pointer, elements, &counter_of_parametres, OPERATOR);
    if (!is_exits)
    {
        ((elements->all_variables)[empty_index]).address = last_address - 4;
        strncpy(((elements->all_variables)[empty_index]).name, ((root->left)->value).variable_name, strlen(((root->left)->value).variable_name));
        ram_address = last_address - 4;
    }
    fprintf(file_pointer, "\tpop rax\n");
    fprintf(file_pointer, "\tmov DWORD [rbp %d], eax\n", ram_address);
    (elements->all_registers)[0].is_free = true;
    return;
}

static void process_expression_after_assignment(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, size_t *counter_of_parametres, Value_type type)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }

    process_expression_after_assignment((root->left), file_pointer, elements, counter_of_parametres, type);
    process_expression_after_assignment((root->right), file_pointer, elements, counter_of_parametres, type);
    switch ((root->value).type)
    {
        case NUMBER:
        {
            if ((root->value).type_of_number == DOUBLE)
            {
                fprintf(file_pointer, "\tpush %f\n", (root->value).number);
            }
            else if ((root->value).type_of_number == INT)
            {
                fprintf(file_pointer, "\tpush %d\n", (int)(root->value).number);
            }
            (*counter_of_parametres)++;
            return;
        }
        case VARIABLE:
        {
            if (!elements->is_body_of_functions)
            {
                process_global_variable(&(root->value), file_pointer, elements);
            }
            else
            {
                process_local_variable(&(root->value), file_pointer, elements);
            }
            (*counter_of_parametres)++;
            return;
        }
        case OPERATION:
        {
            process_operation(&(root->value), file_pointer);
            (*counter_of_parametres)--;
            return;
        }
        case BUILT_IN_FUNCTION:
        {
            process_built_in_function(root, file_pointer, elements);
            (*counter_of_parametres)++;
            return;
        }
        case CALLER_OF_FUNCTION:
        {
            process_caller_of_function(root, file_pointer, elements);
            (*counter_of_parametres)++;
            return;
        }
        case OPERATOR:
        {
            if ((root->value).operator_ == OPERATOR_COMMA)
            {
                //add_parametres_for_function(root->right, file_pointer, all_variables);
                //process_built_in_function(root, file_pointer, all_variables);
                return;
            }
            break;
        }
        default:
        {
            printf("operator = %d\n", (root->value).operator_);
            fprintf(stderr, "ERROR OF UNKNOWN TYPE\n");
            abort();
        }
        
    }
    return;
}

static void process_local_variable(struct Value *value, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (value == NULL || file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING LOCAL VARIABLE\n");
        abort();
    }
    Stack_Elem_t element = 0;
    Errors error = stack_element(&(elements->current_function), &element);
    size_t index = (size_t)element;
    if (error != NO_ERRORS)
    {
        fprintf(stderr, "error of stack = %d\n", error);
        abort();
    }
    //printf("value->variable_name = %s\n", value->variable_name);
    bool is_added = false;
    size_t id = 0;
    for (id = 0; id < SIZE_OF_ALL_VARIABLES; id++)
    {
        //printf("variable name = %s\n", (((elements->all_functions)[index]).all_local_variables)[id].name);
        if (strcasecmp((((elements->all_functions)[index]).all_local_variables)[id].name, value->variable_name) == 0)
        {
            //printf("address = %d\n", (((elements->all_functions)[index]).all_local_variables)[id].address);
            is_added = true;
            break;
        }
        if (strlen((((elements->all_functions)[index]).all_local_variables)[id].name) == 0)
        {
            //printf("new\n");
            is_added = true;
            strncpy((((elements->all_functions)[index]).all_local_variables)[id].name, value->variable_name, strlen(value->variable_name));
            (((elements->all_functions)[index]).all_local_variables)[id].address = ((elements->all_functions)[index]).last_free_address;
            //printf("new address = %d\n", (((elements->all_functions)[index]).all_local_variables)[id].address);
            ((elements->all_functions)[index]).last_free_address -= 4;
            break;
        }
    }
    if (!is_added)
    {
        fprintf(stderr, "Error! There is not enough memory in all_variables array for function %s\n", ((elements->all_functions)[index]).function_name);
        abort();
    }
    if (!((elements->all_functions)[index]).is_parametres_processed)
    {
        return;
    }

    int address_of_variable = (((elements->all_functions)[index]).all_local_variables)[id].address;
    // fprintf(file_pointer, "pop [%d]\n", address_of_variable);
    char str[10] = "";
    char str_for_stack[10] = "";
    if ((elements->all_registers)[0].is_free)
    {
        snprintf(str, 10, "eax");
        snprintf(str_for_stack, 10, "rax");
        (elements->all_registers)[0].is_free = false;
    }
    else
    {
        snprintf(str, 10, "edx");
        snprintf(str_for_stack, 10, "rdx");
        (elements->all_registers)[3].is_free = false;
    }
    fprintf(file_pointer, "\tmov %s, DWORD [rbp %d]\n", str, address_of_variable);
    fprintf(file_pointer, "\tpush %s\n", str_for_stack);
    return;
}

static void process_global_variable(struct Value *value, FILE *file_pointer, struct Special_elements_for_processing *elements)
{
    if (value == NULL || file_pointer == NULL || elements == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING VARIABLE\n");
        abort();
    }
    bool is_exists = false;
    int address = -1;
    for (size_t index = 0; index < SIZE_OF_ALL_VARIABLES; index++)
    {
        //printf("name of variable = %s\n", ((*all_variables)[index]).name);
        if (strcasecmp(value->variable_name, ((elements->all_variables)[index]).name) == 0)
        {
            is_exists = true;
            address = ((elements->all_variables)[index]).address;
            break;
        }
    }

    if (!is_exists)
    {
        fprintf(stderr, "ERROR OF USING UNKNOWN VARIABLE\n");
        abort();
    }
    char str[10] = "";
    char str_for_stack[10] = "";
    if ((elements->all_registers)[0].is_free)
    {
        snprintf(str, 10, "eax");
        snprintf(str_for_stack, 10, "rax");
        (elements->all_registers)[0].is_free = false;
    }
    else
    {
        snprintf(str, 10, "edx");
        snprintf(str_for_stack, 10, "rdx");
        (elements->all_registers)[3].is_free = false;
    }
    fprintf(file_pointer, "\tmov %s, DWORD [rbp %d]\n", str, address);
    fprintf(file_pointer, "\tpush %s\n", str_for_stack);
    return;
}

static void process_operation(struct Value *value, FILE *file_pointer)
{
    if (value == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING OPERATION\n");
        abort();
    }
    fprintf(file_pointer, "\tpop rdx\n\tpop rax\n");
    switch(value->operation)
    {
        case OP_ADD:
        {
            fprintf(file_pointer, "\tadd eax, edx\n");
            break;
        }
        case OP_SUB:
        {
            fprintf(file_pointer, "\tsub eax, edx\n");
            break;
        }
        case OP_MUL:
        {
            fprintf(file_pointer, "\timul eax, edx\n");
            break;
        }
        case OP_DIV:
        {
            fprintf(file_pointer, "\tpush rbx\n\tmov rbx, rdx\n\tmov rdx, 0\n");
            fprintf(file_pointer, "\tidiv ebx\n");
            fprintf(file_pointer, "\tpop rbx\n");
            break;
        }
        case OP_DEG:
        {
            create_deg_operation(file_pointer);
            break;
        }
        default: return;
    }
    fprintf(file_pointer, "\tpush rax\n");
    return;
}

static void create_deg_operation(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        fprintf(stderr, "Error! There is no output file\n");
        abort();
    }
    fprintf(file_pointer, "\tpush rdi\n\tpush rsi\n\tmov rdi, rax\n\tmov rsi, rdx\n\tcall pow\n\tpop rsi\n\tpop rdi\n");
    return;
}

static void process_comparison_operation(struct Value *value, FILE *file_pointer)
{
    if (value == NULL || file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PROCESSING COMPARISON OPERATION\n");
        abort();
    }
    switch(value->comp_operation)
    {
        case OP_MORE:
        {
            fprintf(file_pointer, "\tja ");
            return;
        }
        case OP_MORE_OR_EQUAL:
        {
            fprintf(file_pointer, "\tjae ");
            return;
        }
        case OP_LESS:
        {
            fprintf(file_pointer, "\tjb ");
            return;
        }
        case OP_LESS_OR_EQUAL:
        {
            fprintf(file_pointer, "\tjbe ");
            return;
        }
        case OP_EQUAL:
        {
            fprintf(file_pointer, "\tje ");
            return;
        }
        case OP_NOT_EQUAL:
        {
            fprintf(file_pointer, "\tjne ");
            return;
        }
        default: return;
    }
}

static void process_comparison_expression(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, Programm_operators operator_)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }
    //process_expression_after_comparison(root->left, file_pointer, elements, OPERATION); //all_variables, all_labels);
    size_t counter_of = 0;
    process_expression_after_assignment(root->left, file_pointer, elements, &counter_of, OPERATION);
    fprintf(file_pointer, "\tpop rax\n");
    //process_expression_after_comparison(root->right, file_pointer, elements, OPERATION); //all_variables, all_labels);
    process_expression_after_assignment(root->right, file_pointer, elements, &counter_of, OPERATION);
    fprintf(file_pointer, "\tpop rdx\n\tcmp eax, edx\n");
    process_comparison_operation(&(root->value), file_pointer);
    char str_begin[50] = "";
    char str_end[50] = "";
    size_t counter = 0;
    if (operator_ == OPERATOR_IF)
    {
        strncpy(str_begin, "begin_if", 8);
        strncpy(str_end, "end_if", 6);
        counter = elements->counter_of_if;
    }
    else if (operator_ == OPERATOR_WHILE)
    {
        strncpy(str_begin, "begin_while", 11);
        strncpy(str_end, "end_while", 9);
        counter = elements->counter_of_while;
    }
    fprintf(file_pointer, "%s%lu\n", str_begin, counter);
    size_t index = 0;
    const size_t current_len = strlen(str_begin);
    snprintf(str_begin + current_len, sizeof(str_begin) - current_len, "%lu:", counter);
    //printf("strlen(((*all_labels)[index]).name) = %lu\n", strlen(((*all_labels)[index]).name));
    while (index < SIZE_OF_ALL_VARIABLES && strlen(((elements->all_labels)[index]).name) != 0)
    {
        //printf("index = %lu\n", index);
        if (strcasecmp(str_begin, ((elements->all_labels)[index]).name) == 0)
        {
            index = SIZE_OF_ALL_VARIABLES;
        }
        index++;
    }
    if (index < SIZE_OF_ALL_VARIABLES)
    {
        strncpy(((elements->all_labels)[index]).name, str_begin, strlen(str_begin));
        index++;
    }
    if (((root->parent_node)->node_for_operator_else) == NULL)
    {
        fprintf(file_pointer, "\tjmp %s%lu\n", str_end, counter);
    }
    else
    {
        fprintf(file_pointer, "\tjmp begin_else%lu\n", counter);
        // if (elements->counter_of_else == 0)
        // {
        //     fprintf(file_pointer, "jmp begin_else1:\n");
        //     //(elements->counter_of_else)++;
        //     //Errors error = stack_push(&(elements->stack_else), 1);
        // }
        // else
        // {
        //     Stack_Elem_t element = 0;
        //     Errors error = stack_element(&(elements->stack_else), &element);
        //     if (error != NO_ERRORS)
        //     {
        //         fprintf(stderr, "error of stack = %d\n", error);
        //         abort();
        //     }
        //     fprintf(file_pointer, "jmp begin_else%lu:\n", (size_t)element + 1);
        // }
    }

    return;
}


static void process_expression_after_comparison(struct Node *root, FILE *file_pointer, struct Special_elements_for_processing *elements, Value_type type)
{
    if (root == NULL)
    {
        return;
    }
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF CREATING ASM FILE\n");
        abort();
    }
    process_expression_after_comparison(root->left, file_pointer, elements, type);//all_variables, all_labels);
    process_expression_after_comparison(root->right, file_pointer, elements, type);//all_variables, all_labels);
    switch ((root->value).type)
    {
        case NUMBER:
        {
            fprintf(file_pointer, "\tpush %f\n", (root->value).number);
            return;
        }
        case VARIABLE:
        {
            if (!elements->is_body_of_functions)
            {
                process_global_variable(&(root->value), file_pointer, elements);
            }
            else
            {
                process_local_variable(&(root->value), file_pointer, elements);
            }
            return;
        }
        case OPERATION:
        {
            process_operation(&(root->value), file_pointer);
            return;
        }
        default:
        {
            fprintf(stderr, "ERROR OF UNKNOWN TYPE\n");
            abort();
        }
    }
    return;

}