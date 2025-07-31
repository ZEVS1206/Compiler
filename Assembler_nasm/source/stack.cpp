#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stack.h"
#include "recalloc.h"


#define STACK_ASSERT(stack) stack_assert((stack), __FILE__, __LINE__)
#define STACK_STOP(error) stack_stop((error), __FILE__, __LINE__)


static void new_hash(struct MyStack *stack);
static void hash_check(const struct MyStack *stack, int capacity);
static Errors do_recalloc(struct MyStack *stack, Stack_Elem_t *reserve);
static void stack_stop(Errors error, const char *file, int line);
static Errors stack_assert(const struct MyStack *stack, const char *file, int line);
static void check_canaries(const struct MyStack *stack);

static void check_canaries(const struct MyStack *stack)
{
    if (*(stack->data - 1) != left_canary || *(stack->data + stack->capacity) != right_canary)
    {
        STACK_STOP(CANARY_DETECTED_HACK_OF_STACK);
    }
}


static void new_hash(struct MyStack *stack)
{
    stack->hash_result = hash(stack, stack->capacity);
    return;
}

static void hash_check(const struct MyStack *stack, int capacity)
{
    uint64_t result_of_hash = hash(stack, capacity);
    if (result_of_hash != stack->hash_result)
    {
        STACK_STOP(HASH_DETECTED_HACK_OF_STACK);
    }
    return;
}

uint64_t hash(const struct MyStack *stack, int capacity)
{
    uint64_t result = 5381;
    for (int i = 0; i < capacity; i++)
    {
        result = (result * 31) ^ (uint64_t)(((stack->data)[i] < 0) ? -(stack->data)[i]: (stack->data)[i]);
    }
    return result;
}

static void stack_stop(Errors error, const char *file, int line)
{
    fprintf(stderr, "%s\nfile - %s\nline = %d\n", get_error(error), file, line);
    abort();
    return;
}

static Errors stack_assert(const struct MyStack *stack, const char *file, int line)
{
    Errors error = stack_check(stack);
    if (error != NO_ERRORS)
    {
        STACK_STOP(error);
    }
    return NO_ERRORS;
}

const char* get_error(Errors error)
{
    switch (error)
    {
        case ERROR_OF_NULL_STACK:
            return "ERROR_OF_NULL_STACK";
            break;
        case CANARY_DETECTED_HACK_OF_STACK:
            return "CANARY_DETECTED_HACK_OF_STACK";
            break;
        case ERROR_OF_HASH:
            return "ERROR_OF_HASH";
            break;
        case HASH_DETECTED_HACK_OF_STACK:
            return "HASH_DETECTED_HACK_OF_STACK";
            break;
        case ERROR_OF_ADD_TO_STACK:
            return "ERROR_OF_ADD_TO_STACK";
            break;
        case ERROR_OF_DEL_FROM_STACK:
            return "ERROR_OF_DEL_FROM_STACK";
            break;
        case ERROR_OF_NULL_SIZE:
            return "ERROR_OF_NULL_SIZE";
            break;
        case ERROR_OF_DESTRUCTOR_STACK:
            return "ERROR_OF_DESTRUCTOR_STACK";
            break;
        case ERROR_OF_CHECK_STACK:
            return "ERROR_OF_CHECK_STACK";
            break;
        case ERROR_OF_RECALLOC_STACK:
            return "ERROR_OF_RECALLOC_STACK";
            break;
        case ERROR_OF_STACK_OVERFLOW:
            return "ERROR_OF_STACK_OVERFLOW";
            break;
        case ERROR_OF_USING_DATA:
            return "ERROR_OF_USING_DATA";
            break;
        case NO_ERRORS:
            return "NO_ERRORS";
            break;
        default:
            return "UNKNOWN_TYPE_OF_ERROR";
            break;
    }
}

Errors stack_constructor(struct MyStack *stack, int begin_capacity ON_DEBUG(,const char *name, const char *file, int line))
{
    stack->data = (Stack_Elem_t *) calloc((size_t)begin_capacity + 2, sizeof(Stack_Elem_t));
    /*if (stack->data == NULL) */

    *(stack->data) = left_canary;
    stack->LEFT_CANARY = left_canary;
    *(stack->data + begin_capacity + 1) = right_canary;
    stack->RIGHT_CANARY = right_canary;

    stack->data += 1;
    stack->size  = 0;
    stack->capacity = begin_capacity;
    stack->old_capacity = stack->capacity;
    #ifdef FULL_DEBUG
    stack->name = name;
    stack->file = file;
    stack->line = line;
    #endif
    //hash_protect(stack, stack->capacity);
    new_hash(stack);
    check_canaries(stack);
    Errors error = STACK_ASSERT(stack);
    STACK_DUMP(stack);
    return error;
}

Errors stack_destructor(struct MyStack *stack)
{
    if (stack->data == NULL)
    {
        STACK_STOP(ERROR_OF_DESTRUCTOR_STACK);
    }
    for (int i = 0; i < stack->size; i++)
    {
        (stack->data)[i] = 0;
    }

    stack->size = 0;
    stack->capacity = 0;
    // printf("|||||||||||\n");
    // stack_dump(stack);
    // printf("|||||||||||\n");
    free(stack->data - 1);
    return NO_ERRORS;
}


static Errors do_recalloc(struct MyStack *stack, Stack_Elem_t *reserve)
{
    Errors error = STACK_ASSERT(stack);
    reserve = stack->data;
    Stack_Elem_t *copy = (Stack_Elem_t *) calloc((size_t)stack->capacity + 2, sizeof(Stack_Elem_t));
    memcpy(copy, stack->data - 1, (size_t)stack->capacity + 2);
    *(stack->data + stack->old_capacity) = 0;
    stack->data = (Stack_Elem_t *) recalloc((stack->data - 1), (size_t)stack->capacity + 2, sizeof(Stack_Elem_t), size_t(stack->old_capacity));
    if (stack->data == NULL)
    {
        stack->data = reserve;
        STACK_DUMP(stack);
        STACK_STOP(ERROR_OF_RECALLOC_STACK);
    }
    memcpy(stack->data, copy, (size_t)stack->capacity);
    free(copy);
    stack->old_capacity = stack->capacity;
    //*(stack->data) = (const Stack_Elem_t)0xDEADA;
    // printf("old_capacity=%d\n\n", old_capacity);
    // printf("new_capacity=%d\n\n", stack->capacity);
    // printf("capacity=%d\n\n", abs(stack->capacity - old_capacity));
    //printf("*(stack->data + stack->capacity)=%f\n", *(stack->data + stack->capacity));
    *(stack->data + stack->capacity + 1) = right_canary;
    *(stack->data) = left_canary;
    // printf("STACK:\n");
    // for (int i = 0; i < stack->capacity + 2; i++)
    // {
    //     printf("%f ", (stack->data)[i]);
    // }
    // printf("\n\n");
    stack->data = stack->data + 1;
    //hash_protect(stack, stack->capacity);
    new_hash(stack);
    check_canaries(stack);
    error = STACK_ASSERT(stack);
    return error;
}

Errors stack_check(const struct MyStack *stack)
{
    if (stack->data == NULL)
    {
        return ERROR_OF_USING_DATA;
    }
    if (stack->size > stack->capacity)
    {
        return ERROR_OF_RECALLOC_STACK;
    }
    if (stack->size < 0)
    {
        return ERROR_OF_NULL_SIZE;
    }
    return NO_ERRORS;
}

Errors stack_element(struct MyStack *stack, Stack_Elem_t *element)
{
    hash_check(stack, stack->capacity);
    Errors error = STACK_ASSERT(stack);
    if (stack->size - 1 < 0)
    {
        return ERROR_OF_NULL_SIZE;
    }
    *element = (stack->data)[stack->size - 1];
    //printf("Last element in stack = %lf\n", *element);
    error = STACK_ASSERT(stack);
    hash_check(stack, stack->capacity);
    return error;
}

Errors stack_push(struct MyStack *stack, Stack_Elem_t element)
{
    //hash_protect(stack, stack->capacity);
    hash_check(stack, stack->capacity);
    Errors error = STACK_ASSERT(stack);
    (stack->data)[stack->size] = element;
    new_hash(stack);
    if (stack->size + 1 >= stack->capacity)
    {
        stack->old_capacity = stack->capacity;
        //hash_protect(stack, stack->old_capacity);
        hash_check(stack, stack->old_capacity);
        (stack->capacity) *= COEFFICIENT;
        Stack_Elem_t *reserve = NULL;
        error = do_recalloc(stack, reserve);
        if (error != NO_ERRORS)
        {
            STACK_DUMP(stack);
            STACK_STOP(ERROR_OF_ADD_TO_STACK);
        }
    }
    (stack->size)++;
    //hash_protect(stack, stack->capacity);
    hash_check(stack, stack->capacity);
    check_canaries(stack);
    // for (int i = 0; i < stack->capacity; i++)
    // {
    //     printf("%f ", (stack->data)[i]);
    // }
    // printf("\n\n");
    STACK_DUMP(stack);
    error = STACK_ASSERT(stack);
    return error;
}

Errors stack_pop(struct MyStack *stack, Stack_Elem_t *element)
{
    hash_check(stack, stack->capacity);
    if (stack->size - 1 < 0)
    {
        STACK_STOP(ERROR_OF_NULL_SIZE);
    }
    Errors error = STACK_ASSERT(stack);
    *element = (stack->data)[stack->size - 1];
    LESS_DEBUG(printf("last_element=%lf\n\n", *element);)
    (stack->data)[stack->size - 1] = 0;
    (stack->size)--;
    new_hash(stack);
    if ((stack->capacity) / COEFFICIENT >= (stack->size + 1))
    {
        stack->old_capacity = stack->capacity;
        //hash_protect(stack, stack->old_capacity);
        hash_check(stack, stack->old_capacity);
        stack->capacity /= COEFFICIENT;
        Stack_Elem_t *reserve = NULL;
        error = do_recalloc(stack, reserve);
        if (error != NO_ERRORS)
        {
            STACK_DUMP(stack);
            STACK_STOP(ERROR_OF_DEL_FROM_STACK);
        }
    }
    hash_check(stack, stack->capacity);
    //hash_protect(stack, stack->capacity);
    check_canaries(stack);
    STACK_DUMP(stack);
    error = STACK_ASSERT(stack);
    return error;
}

void stack_dump(struct MyStack *stack ON_DEBUG(,const char *file, int line))
{
    LESS_DEBUG(printf("||||||||||||||||||||||||\n");)
    LESS_DEBUG(printf("Information about stack:\n");)
    ON_DEBUG(printf("Stack address: %p\n", stack));
    if (stack != NULL)
    {
        ON_DEBUG(printf("Name: %s\n", stack->name);)
        ON_DEBUG(printf("File: %s\n", file);)
        ON_DEBUG(printf("Line: %d\n", line);)
        ON_DEBUG(printf("Data address: %p\n", &(stack->data));)
        LESS_DEBUG(printf("Size value: %d\n", stack->size);)
        LESS_DEBUG(printf("Hash result: %lu\n", stack->hash_result);)
        LESS_DEBUG(printf("Capacity value: %d\n", stack->capacity);)
        LESS_DEBUG(printf("LEFT_CANARY: %lf\n", *(stack->data - 1));)
        LESS_DEBUG(printf("RIGHT_CANARY: %lf\n", *(stack->data + stack->capacity));)
        if (stack->data != NULL)
        {
            LESS_DEBUG(printf("Stack:\n");)
            LESS_DEBUG(stack->data -= 1;)
            LESS_DEBUG(for (int i = 0; i < stack->capacity + 2; i++))
            LESS_DEBUG({)
                LESS_DEBUG(printf("%lf ", (stack->data)[i]);)
            LESS_DEBUG(})
            LESS_DEBUG(stack->data += 1;)
        }
        LESS_DEBUG(printf("\n");)
    }
    LESS_DEBUG(printf("||||||||||||||||||||||||\n");)
    LESS_DEBUG(printf("\n\n");)
    return;
}

void special_dump(struct MyStack *stack)
{
    if (stack != NULL)
    {
        if (stack->data != NULL)
        {
        printf("Stack:\n");
        for (int i = 0; i < stack->size; i++)
        {
            printf("%lf ", (stack->data)[i]);
        }
        printf("\n\n");
        }
    }
    return;
}


