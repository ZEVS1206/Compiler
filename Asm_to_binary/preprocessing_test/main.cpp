#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STATIC_LEN 1000

int preprocess_programm(FILE *source, FILE *add, FILE *result);
static size_t get_size_of_file(FILE *file_pointer);

static size_t get_size_of_file(FILE *file_pointer)
{
    fseek(file_pointer, 0L, SEEK_END);
    size_t size_of_file = (size_t)ftell(file_pointer);
    fseek(file_pointer, 0L, SEEK_SET);
    return size_of_file;
}

int preprocess_programm(FILE *source, FILE *add, FILE *result)
{
    if (source == NULL || add == NULL || result == NULL)
    {
        return 1;
    }
    size_t size_of_source = get_size_of_file(source);
    size_t size_of_add    = get_size_of_file(add);
    char *buffer_with_commands_from_source = (char *) calloc (size_of_source, sizeof(char));
    if (buffer_with_commands_from_source == NULL)
    {
        return 1;
    }
    size_t result_of_reading = fread(buffer_with_commands_from_source, sizeof(char), size_of_source, source);
    if (result_of_reading != size_of_source)
    {
        return 1;
    }
    fclose(source);
    char *buffer_with_commands_from_add = (char *) calloc (size_of_add + 1, sizeof(char));
    if (buffer_with_commands_from_add == NULL)
    {
        return 1;
    }
    result_of_reading = fread(buffer_with_commands_from_add, sizeof(char), size_of_add, add);
    if (result_of_reading != size_of_add)
    {
        return 1;
    }
    buffer_with_commands_from_add[size_of_add] = '\0';
    fclose(add);
    size_t size_of_result = (size_of_add + size_of_source) * 2;
    char *buffer_with_out_commands = (char *) calloc (size_of_result, sizeof(char));
    if (buffer_with_out_commands == NULL)
    {
        return 1;
    }

    size_t position_in_result = 0;
    for (size_t pos_in_source = 0; pos_in_source < size_of_source; ++pos_in_source)
    {
        // if (position_in_result >= size_of_result)
        // {
        //     break;
        // }
        if (isspace(buffer_with_commands_from_source[pos_in_source]))
        {
            buffer_with_out_commands[position_in_result++] = buffer_with_commands_from_source[pos_in_source];
            continue;
        }
        char line[STATIC_LEN] = "";
        size_t index_line = 0;
        size_t pos_for_get_line = pos_in_source;
        while (index_line < STATIC_LEN && pos_for_get_line < size_of_source && buffer_with_commands_from_source[pos_for_get_line] != ' ')
        {
            line[index_line] = buffer_with_commands_from_source[pos_for_get_line];
            index_line++;
            pos_for_get_line++;
        }
        if (strcasecmp(line, "%include") == 0)
        {
            while (pos_for_get_line < size_of_source && buffer_with_commands_from_source[pos_for_get_line] != '\n')
            {
                pos_for_get_line++;
            }
            pos_in_source = pos_for_get_line;
            continue;
        }
        if (buffer_with_commands_from_source[pos_in_source] == ';')
        {
            index_line = 0;
            memset(line, '\0', strlen(line));
            pos_for_get_line = pos_in_source;
            while (index_line < STATIC_LEN && pos_for_get_line < size_of_source && buffer_with_commands_from_source[pos_for_get_line] != '\n')
            {
                line[index_line] = buffer_with_commands_from_source[pos_for_get_line];
                index_line++;
                pos_for_get_line++;
            }
            if (index_line < STATIC_LEN &&  pos_for_get_line < size_of_source)
            {
                line[index_line] = buffer_with_commands_from_source[pos_for_get_line];
            }

            snprintf(buffer_with_out_commands + position_in_result, strlen(line) + 1, "%s", line);
            //printf("%s\n", buffer_with_out_commands);
            position_in_result += strlen(line);
            pos_in_source = pos_for_get_line;
            continue;
        }
        if (strcasecmp(line, "global") == 0)
        {
            while (index_line < STATIC_LEN && pos_for_get_line < size_of_source && buffer_with_commands_from_source[pos_for_get_line] != '\n')
            {
                line[index_line++] = buffer_with_commands_from_source[pos_for_get_line++];
            }
            if (index_line < STATIC_LEN && pos_for_get_line < size_of_source)
            {
                line[index_line] = buffer_with_commands_from_source[pos_for_get_line];
            }
            pos_in_source = pos_for_get_line;
            if (strcasecmp(line, "global _start\n") == 0)
            {
                snprintf(buffer_with_out_commands + position_in_result, strlen(line) + 1, "%s", line);
                //printf("%s\n", buffer_with_out_commands);
                position_in_result += strlen(line);
                // printf("line = %s\nstrlen(line) = %lu\n", line, strlen(line));
                // printf("position_in_result = %lu\n", position_in_result);
                // printf("buffer_add[0] = %s\n", buffer_with_commands_from_add);
                //printf("add = %s\n", buffer_with_commands_from_add);
                snprintf(buffer_with_out_commands + position_in_result, size_of_add + 1, "%s", buffer_with_commands_from_add);
                position_in_result += size_of_add;
            }
            else
            {
                fprintf(stderr, "Error! Incorrect information in source file!\n");
                abort();
            }
            continue;
        }
        memset(line, '\0', strlen(line));
        index_line = 0;
        pos_for_get_line = pos_in_source;
        while (index_line < STATIC_LEN && pos_for_get_line < size_of_source && buffer_with_commands_from_source[pos_for_get_line] != '\n')
        {
            line[index_line++] = buffer_with_commands_from_source[pos_for_get_line++];
        }
        if (index_line < STATIC_LEN && pos_for_get_line < size_of_source)
        {
            line[index_line] = buffer_with_commands_from_source[pos_for_get_line];
        }
        snprintf(buffer_with_out_commands + position_in_result, strlen(line) + 1, "%s", line);
        //printf("%s\n", buffer_with_out_commands);
        position_in_result += strlen(line);
        pos_in_source = pos_for_get_line;
    }

    size_t result_of_writing = fwrite(buffer_with_out_commands, sizeof(char), strlen(buffer_with_out_commands), result);
    

    free(buffer_with_commands_from_source);
    free(buffer_with_commands_from_add);
    free(buffer_with_out_commands);
    return 0;
}

int main()
{
    const char *source_file_name = "source.txt";
    const char *file_with_additional = "additional.txt";
    const char *source_out_file_name = "result.txt";
    FILE *source = fopen(source_file_name, "rb");
    FILE *result = fopen(source_out_file_name, "w");
    FILE *add    = fopen(file_with_additional, "rb");
    int res = preprocess_programm(source, add, result);
    if (res != 0)
    {
        fprintf(stderr, "Error of preprocessing\n");
        return 1;
    }
    fclose(result);
    return 0;
}