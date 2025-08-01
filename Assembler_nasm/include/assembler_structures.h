#ifndef ASSEMBLER_STRUCTURES_H
#define ASSEMBLER_STRUCTURES_H

#define SIZE_OF_ALL_VARIABLES 50
#define SIZE_OF_ALL_REGISTERS 16
const int TOXIC = 1985;
const long long int RAM_SIZE  = 500;

#include "../../Reader/include/tree.h"
#include "stack.h"

enum Registers
{
    NOT_A_REGISTER = 0,
    RAX            = 1,
    RBX            = 2,
    RCX            = 3,
    RDX            = 4,
    RSI            = 5,
    RDI            = 6,
    RBP            = 7,
    RSP            = 8,
    R8             = 9,
    R9             = 10,
    R10            = 11,
    R11            = 12,
    R12            = 13,
    R13            = 14,
    R14            = 15,
    R15            = 16
};

enum Errors_of_ASM
{
    NO_ASM_ERRORS              = 0,
    ERROR_OF_UNKNOWN_TEXT_CMD  = 1,
    ERROR_OF_READING_FROM_FILE = 2,
    ERROR_OF_NO_COMMANDS       = 3,
    ERROR_OF_CREATING_OUT_FILE = 4,
    ERROR_OF_DESTRUCTOR_ASM    = 5,
    ERROR_OF_CREATE_ARRAY      = 6,
    ERROR_OF_UNKNOWN_REGISTER  = 7,
    ERROR_OF_CONSTRUCTOR_ASM   = 8,
    ERROR_OF_PARSE_WORD        = 9,
    ERROR_OF_CREATE_ASM_FILE   = 10,
    ERROR_OF_OPERATING_TREE      = 11
};

struct Register
{
    Registers name;
    bool is_free;
};

struct Label
{
    char name[50];
    int address;
};

struct Labels
{
    char name[50];
};

struct Function_type
{
    char function_name[100];
    struct Label *all_local_variables;
    size_t start_local_memory_address;
    size_t end_local_memory_address;
    bool is_parametres_processed;
};

struct Special_elements_for_processing
{
    struct MyStack current_function;
    size_t counter_of_if;
    size_t counter_of_while;
    size_t counter_of_else;
    size_t start_local_memory_address;
    struct MyStack stack_if;
    struct MyStack stack_while;
    struct MyStack stack_else;
    struct MyStack stack_for_operations;
    struct Label *all_variables;
    struct Labels *all_labels;
    struct Function_type *all_functions;
    struct Register *all_registers;
    bool is_body_of_functions;
    bool is_assignment;
};

Errors_of_ASM transform_programm_to_assembler(struct Tree *tree, struct Labels **all_labels);



#endif