#include <stdio.h>
#include <stdlib.h>

#include "assembler_structures.h"


int main(int argc, char *argv[])
{
    struct Tree tree = {0};
    const char *file_name = argv[1];
    Errors_of_tree error = tree_reader(&tree, file_name); //"../Reader/source/input.txt");
    if (error != NO_ERRORS_TREE)
    {  
        fprintf(stderr, "error of tree = %d\n", error);
        abort();
    }
    struct Labels *all_labels = (struct Labels *) calloc(SIZE_OF_ALL_VARIABLES, sizeof(struct Labels));
    if (all_labels == NULL)
    {
        fprintf(stderr, "ERROR OF CREATE ARRAY OF LABELS\n");
        abort();
    }
    Errors_of_ASM error_asm = transform_programm_to_assembler(&tree, &all_labels);
    if (error_asm != NO_ASM_ERRORS)
    {
        fprintf(stderr, "error_asm = %d\n", error_asm);
        abort();
    }
    free(all_labels);
    error = tree_destructor(&tree);
    if (error != NO_ERRORS_TREE)
    {
        fprintf(stderr, "error of tree = %d\n", error);
        return 1;
    }
}