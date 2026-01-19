#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/prepocessing_header.h"

int preprocess_programm(FILE *source, FILE *add, FILE *result);
static size_t get_size_of_file(FILE *file_pointer);
void get_all_macros(char *buffer, size_t size_of_buffer, struct Macro_type *macros, size_t size_of_macros);
int parse_source_and_add_files(char *buffer_with_commands_from_source, size_t size_of_source,
                               char *buffer_with_commands_from_add,    size_t size_of_add,  
                               char *buffer_with_out_commands,         size_t size_of_result,
                               struct Macro_type *macros,              size_t size_of_macros);

static size_t get_size_of_file(FILE *file_pointer)
{
    fseek(file_pointer, 0L, SEEK_END);
    size_t size_of_file = (size_t)ftell(file_pointer);
    fseek(file_pointer, 0L, SEEK_SET);
    return size_of_file;
}

void get_all_macros(char *buffer, size_t size_of_buffer, struct Macro_type *macros, size_t size_of_macros)
{
    for (size_t index = 0; index < size_of_buffer; ++index)
    {
        if (buffer[index] != '%')
        {
            continue;
        }
        char info_of_macro[STATIC_LEN_FOR_MIDDLE_ARRAYS];
        memset(info_of_macro, 0, STATIC_LEN_FOR_MIDDLE_ARRAYS);
        size_t pos = 0;
        while (buffer[index] != ' ')
        {
            info_of_macro[pos++] = buffer[index++];
        }
        //printf("info of macro = %s\n", info_of_macro);
        if (strcasecmp(info_of_macro, "%macro") != 0)
        {
            continue;
        }
        memset(info_of_macro, 0, STATIC_LEN_FOR_MIDDLE_ARRAYS);
        pos = 0;
        index++;
        
        while (!isspace(buffer[index]))
        {
            info_of_macro[pos++] = buffer[index++];
        }
        //printf("name_of_macro = %s\n", info_of_macro);
        size_t id = 0;
        for (; id < size_of_macros; ++id)
        {
            if (strlen(macros[id].name_of_macro) == 0)
            {
                strcpy(macros[id].name_of_macro, info_of_macro);
                break;
            }
        }
        index++;
        memset(info_of_macro, 0, STATIC_LEN_FOR_MIDDLE_ARRAYS);
        pos = 0;
        while (!isspace(buffer[index]))
        {
            info_of_macro[pos++] = buffer[index++];
        }
        index++;
        int count_of_arguments = atoi(info_of_macro);
        macros[id].count_of_arguments = (size_t)count_of_arguments;
        macros[id].arguments = (char **) calloc (count_of_arguments, sizeof(char *));
        if (macros[id].arguments == NULL)
        {
            fprintf(stderr, "Error of processing macro %s\n", macros[id].name_of_macro);
            abort();
        }
        memset(info_of_macro, 0, STATIC_LEN_FOR_MIDDLE_ARRAYS);
        pos = 0;
        while (index < size_of_buffer && pos < STATIC_LEN_FOR_MIDDLE_ARRAYS)
        {
            if (buffer[index] == '%')
            {
                if (buffer[index + 1] == 'e')
                {
                    break;
                }
            }
            //printf("%c", buffer[index]);
            info_of_macro[pos++] = buffer[index++];
        }
        //printf("info of macro = %s\n", info_of_macro);
        //printf("len(info_of_macro) = %lu\n", strlen(info_of_macro));
        strcpy(macros[id].body_of_macro, info_of_macro);
        macros[id].size_of_body_of_macro = pos - 1;
    }
    for (size_t id = 0; id < size_of_macros; ++id)
    {
        if (strlen(macros[id].name_of_macro) == 0)
        {
            break;
        }
        printf("%s\n", macros[id].name_of_macro);
        printf("%s\n", macros[id].body_of_macro);
        //printf("count_of_arguments = %lu\n", macros[id].count_of_arguments);
    }
    return;
}

int parse_source_and_add_files(char *buffer_with_commands_from_source, size_t size_of_source,
                               char *buffer_with_commands_from_add,    size_t size_of_add,  
                               char *buffer_with_out_commands,         size_t size_of_result,
                               struct Macro_type *macros,              size_t size_of_macros)
{
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
        bool it_is_macro = false;
        size_t index_in_macros = 0;
        for (index_in_macros = 0; index_in_macros < size_of_macros; ++index_in_macros)
        {
            if (strcasecmp(line, macros[index_in_macros].name_of_macro) == 0)
            {
                it_is_macro = true;
                break;
            }
        }
        if (it_is_macro)
        {
            pos_for_get_line++;
            for (size_t index_in_arguments = 0; index_in_arguments < macros[index_in_macros].count_of_arguments; ++index_in_arguments)
            {
                index_line = 0;
                memset(line, '\0', strlen(line));
                while (index_line < STATIC_LEN && pos_for_get_line < size_of_source && !isspace(buffer_with_commands_from_source[pos_for_get_line])
                       && buffer_with_commands_from_source[pos_for_get_line] != ',')
                {
                    line[index_line] = buffer_with_commands_from_source[pos_for_get_line];
                    index_line++;
                    pos_for_get_line++;
                }
                if (buffer_with_commands_from_source[pos_for_get_line] == ',')
                {
                    pos_for_get_line += 2;
                }
                (macros[index_in_macros]).arguments[index_in_arguments] = (char *) calloc (index_line + 1, sizeof(char));
                if (!(macros[index_in_macros]).arguments[index_in_arguments])
                {
                    return 1;
                }
                strncpy((macros[index_in_macros]).arguments[index_in_arguments], line, index_line + 1);
            }
            for (size_t index_in_body = 0; index_in_body < (macros[index_in_macros]).size_of_body_of_macro; ++index_in_body)
            {
                if ((macros[index_in_macros]).body_of_macro[index_in_body] != '%')
                {
                    buffer_with_out_commands[position_in_result++] = (macros[index_in_macros]).body_of_macro[index_in_body];
                    continue;
                }
                index_in_body++;
                int argument = (macros[index_in_macros]).body_of_macro[index_in_body] - '0';
                snprintf(buffer_with_out_commands + position_in_result, strlen((macros[index_in_macros]).arguments[argument - 1]) + 1, "%s", (macros[index_in_macros]).arguments[argument - 1]);
                position_in_result += strlen((macros[index_in_macros]).arguments[argument - 1]);
            }
            buffer_with_out_commands[position_in_result++] = '\n';
            pos_in_source = pos_for_get_line;
            continue;
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
    return 0;
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
    struct Macro_type *macros = (struct Macro_type *) calloc (STATIC_LEN_FOR_SMALL_ARRAYS, sizeof(struct Macro_type));
    if (macros == NULL)
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
    get_all_macros(buffer_with_commands_from_add, size_of_add, macros, STATIC_LEN_FOR_SMALL_ARRAYS);
    size_t size_of_result = (size_of_add + size_of_source) * 2;
    char *buffer_with_out_commands = (char *) calloc (size_of_result, sizeof(char));
    if (buffer_with_out_commands == NULL)
    {
        return 1;
    }
    int code = parse_source_and_add_files(buffer_with_commands_from_source, size_of_source,
                                          buffer_with_commands_from_add,    size_of_add,
                                          buffer_with_out_commands,         size_of_result,
                                          macros, STATIC_LEN_FOR_SMALL_ARRAYS);
    if (code)
    {
        return code;
    }


    size_t result_of_writing = fwrite(buffer_with_out_commands, sizeof(char), strlen(buffer_with_out_commands), result);
    

    free(buffer_with_commands_from_source);
    free(buffer_with_commands_from_add);
    free(buffer_with_out_commands);
    for (size_t id = 0; id < STATIC_LEN_FOR_SMALL_ARRAYS; ++id)
    {
        if (macros[id].arguments)
        {
            size_t index = 0;
            while (index < macros[id].count_of_arguments && macros[id].arguments[index])
            {
                free(macros[id].arguments[index++]);
            }
            free(macros[id].arguments);
        }
    }
    free(macros);
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