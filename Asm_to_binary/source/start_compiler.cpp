#include <stdio.h>

#include "function_and_structures.h"

int main()
{
    FILE *file_pointer = fopen("test_nasm/test.asm", "rb");
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF READING CMDS\n");
        return 1;
    }
    Errors_of_binary error = transform_asm_to_binary(file_pointer);
    if (error != NO_BINARY_ERRORS)
    {
        fprintf(stderr, "error = %d\n", error);
        return 1;
    }
    return 0;
}