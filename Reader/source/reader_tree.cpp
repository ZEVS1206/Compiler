#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "tree_dump.h"
#include "tree_input.h"

Errors_of_tree tree_reader(struct Tree *tree, const char *file_name)
{
    Errors_of_tree error = NO_ERRORS_TREE;
    error = tree_constructor(tree);
    if (error != NO_ERRORS_TREE)
    {
        return error;
    }
    error = get_tree_from_file(tree, file_name); //"../../test_file.rt");
    if (error != NO_ERRORS_TREE)
    {
        return error;
    }
    if (((tree->root)->value).type == UNKNOWN_TYPE)
    {
        return ERROR_OF_READ_FROM_FILE;
    }
    tree->tmp_root = tree->root;
    tree->error = NO_ERRORS_TREE;
    graphic_dump(tree, "example");
    if (tree->error != NO_ERRORS_TREE)
    {
        return tree->error;
    }
    // error = tree_destructor(tree);
    // if (error != NO_ERRORS_TREE)
    // {
    //     return error;
    // }
    return NO_ERRORS_TREE;
}

// int main()
// {
//     struct Tree tree = {0};
//     Errors_of_tree error = tree_reader(&tree, "source/input.txt");
//     if (error != NO_ERRORS_TREE)
//     {
//         fprintf(stderr, "error = %d\n", error);
//         return 1;
//     }
//     error = tree_destructor(&tree);
//     if (error != NO_ERRORS_TREE)
//     {
//         fprintf(stderr, "error = %d\n", error);
//         return 1;
//     }
//     return 0;
// }
