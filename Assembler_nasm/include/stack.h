#ifndef STACK_H
#define STACK_H

#include <stdarg.h>
#include <stdint.h>
const int COEFFICIENT = 2;

typedef double Stack_Elem_t;
typedef double Stack_Canary_t;

//#define DEBUG
//#define FULL_DEBUG

#ifdef DEBUG
#define LESS_DEBUG(...) __VA_ARGS__
    #ifdef FULL_DEBUG
        #define ON_DEBUG(...) __VA_ARGS__
    #else
        #define ON_DEBUG(...)
    #endif
#else
#define ON_DEBUG(...)
#define LESS_DEBUG(...)
#endif

const Stack_Canary_t left_canary = 0xDEADA;
const Stack_Canary_t right_canary = 0xDEADC;


#define STACK_CTOR(stack, begin_capacity) stack_constructor((stack), (begin_capacity) ON_DEBUG(,#stack, __FILE__, __LINE__))
#define STACK_DUMP(stack) stack_dump((stack) ON_DEBUG(,__FILE__, __LINE__))



enum Errors
{
    ERROR_OF_NULL_STACK           = -12,
    CANARY_DETECTED_HACK_OF_STACK = -11,
    ERROR_OF_HASH                 = -10,
    HASH_DETECTED_HACK_OF_STACK   = -9,
    ERROR_OF_ADD_TO_STACK         = -8,
    ERROR_OF_DEL_FROM_STACK       = -7,
    ERROR_OF_NULL_SIZE            = -6,
    ERROR_OF_DESTRUCTOR_STACK     = -5,
    ERROR_OF_CHECK_STACK          = -4,
    ERROR_OF_RECALLOC_STACK       = -3,
    ERROR_OF_STACK_OVERFLOW       = -2,
    ERROR_OF_USING_DATA           = -1,
    NO_ERRORS                     =  0
};

struct MyStack
{
    Stack_Canary_t LEFT_CANARY;
    #ifdef FULL_DEBUG
    const char *name;
    const char *file;
    int line;
    #endif
    Stack_Elem_t *data;
    int size;
    int capacity;
    int old_capacity;
    uint64_t hash_result;
    Stack_Canary_t RIGHT_CANARY;
};

Errors stack_constructor(struct MyStack *stack, int begin_capacity ON_DEBUG(,const char *name, const char *file, int line));
void stack_dump         (struct MyStack *stack ON_DEBUG(,const char *file, int line));
void special_dump(struct MyStack *stack);
Errors stack_destructor(struct MyStack *stack);
Errors stack_check(const struct MyStack *stack);
Errors stack_element(struct MyStack *stack, Stack_Elem_t *element);
Errors stack_push(struct MyStack *stack, Stack_Elem_t element);
Errors stack_pop(struct MyStack *stack, Stack_Elem_t *element);
uint64_t hash (const struct MyStack *stack, int capacity);
const char *get_error(Errors error);

#endif
