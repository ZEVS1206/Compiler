#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "recalloc.h"

void* recalloc(void *pointer, size_t new_size, size_t size_of_type, size_t old_size)
{
    void *ptr = realloc(pointer, new_size * size_of_type);
    size_t fill_bytes = (new_size > old_size) ? new_size - old_size : new_size;
    memset(ptr, 0, fill_bytes);
    return ptr;
}
