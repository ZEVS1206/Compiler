#ifndef PREPROCESSING_HEADER_H
#define PREPROCESSING_HEADER_H

#define STATIC_LEN 1000
#define STATIC_LEN_FOR_SMALL_ARRAYS 50
#define STATIC_LEN_FOR_MIDDLE_ARRAYS 300

struct Macro_type
{
    char name_of_macro[STATIC_LEN_FOR_SMALL_ARRAYS];
    char **arguments;
    char body_of_macro[STATIC_LEN_FOR_MIDDLE_ARRAYS];
    size_t count_of_arguments;
    size_t size_of_body_of_macro;
};

int preprocess_programm(FILE *source, FILE *add, FILE *result);

#endif