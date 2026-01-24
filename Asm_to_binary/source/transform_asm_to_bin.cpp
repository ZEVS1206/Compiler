#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <elf.h>

#include "function_and_structures.h"
#include "prepocessing_header.h"

#define MAX_LEN_FOR_STATIC_ARRAYS 100

// const uint8_t buffer_hex[] = {{0x00},
//                               {0x01},
//                               {0x02},
//                               {0x03},
//                               {0x04},
//                               {0x05},
//                               {0x06},
//                               {0x07},
//                               {0x08},
//                               {0x09},
//                               {0x0A},
//                               {0x0B},
//                               {0x0C},
//                               {0x0D},
//                               {0x0E},
//                               {0x0F}
//                              };
// const size_t size_of_buffer_hex_to_bin = sizeof(buffer_hex) / sizeof(uint8_t);

// const struct CMD cmd_to_byte[] = {{"mov", {0xB0}},
//                                   {"rax", {0x08}},
//                                   {"ret", {0x3C}}
//                                  };
// const size_t size_of_cmd_to_byte = sizeof(cmd_to_byte) / sizeof(struct CMD);

static size_t align_up(size_t offset, size_t base);//Need for align_up offset in memory
static unsigned add_string_to_buffer(char *buffer, size_t *len_of_buffer, const char *string);//Function for adding string to buffer
static Errors_of_binary create_binary_file(struct Binary_file *binary_struct);
static Errors_of_binary create_bin_data_for_binary_file(struct Binary_file *binary_struct, FILE *file_pointer_with_commands);
static void get_all_functions(struct Binary_file *binary_struct, FILE *file_pointer_with_commands);
static size_t get_size_of_file(FILE *file_pointer);
static Register get_register(const char *register_name);
static Initial_instructions get_initial_instruction(const char *instruction);
static void add_label(struct Data_CMDS *commands, const char *label_name, int32_t pc, Sections section, size_t size_of_data, Type_of_label type, int64_t imm_data);
static int find_label_and_change_it(struct Data_CMDS *commands, const char *label_name, size_t size_of_data, Sections section, Type_of_label type, int64_t imm_data);
static struct Label get_label(struct Data_CMDS *commands, const char *label_name, Sections section);
static void encode_text_instructions(struct Data_CMDS *commands, struct Binary_file *binary_struct);
static void encode_data_instructions(struct Data_CMDS *commands, struct Binary_file *binary_struct);
static bool check_if_function(struct Binary_file *binary_struct, const char *label_name);
static bool is_str_digit(const char *str);
static char * skip_spaces(char *buffer, char *end_pointer);
static void parse_section_text(struct Binary_file *binary_struct, struct Data_CMDS *commands, FILE *file_pointer_with_commands, size_t size_of_file);
static void parse_instruction_from_text(char *position, struct Instruction *instruction, char *end_pointer, int32_t *pc, struct Data_CMDS *commands);
static void parse_section_data(struct Binary_file *binary_struct, struct Data_CMDS *commands, FILE *file_pointer_with_commands, size_t size_of_file);
static void parse_instruction_from_data(char *position, struct Instruction *instruction, char *end_pointer, int32_t *pc, struct Data_CMDS *commands);
static void add_relocation(struct Binary_file *binary_struct, size_t offset_in_text, const char *symbol_name, uint32_t type, int64_t additional_offset);
static size_t next_power_of_two(size_t n);
static FILE * preprocess_programm_from_source(FILE *file_pointer);


static char * skip_spaces(char *buffer, char *end_pointer)
{
    if (buffer == end_pointer)
    {
        return buffer;
    }
    while (buffer < end_pointer && isspace(*buffer))
    {
        buffer++;
    }
    return buffer;
}


static size_t get_size_of_file(FILE *file_pointer)
{
    fseek(file_pointer, 0L, SEEK_END);
    size_t size_of_file = (size_t)ftell(file_pointer);
    fseek(file_pointer, 0L, SEEK_SET);
    return size_of_file;
}


static size_t next_power_of_two(size_t n) 
{
    if (n == 0) 
    {
        return 1;
    }
    
    if ((n & (n - 1)) == 0) 
    {
        return n;
    }
    
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    
    if (sizeof(size_t) > 4) 
    {
        n |= n >> 32;
    }
    
    n++;
    return n;
}

static size_t align_up(size_t offset, size_t base)
{
    if (base == 0)
    {
        return offset;
    }
    size_t mask = base - 1;
    return (offset + mask) & (~mask);
}

static unsigned add_string_to_buffer(char *buffer, size_t *len_of_buffer, const char *string)
{
    unsigned pos = (unsigned)(*len_of_buffer);
    size_t len_of_str = strlen(string) + 1;
    memcpy(buffer + *len_of_buffer, string, len_of_str);
    *len_of_buffer += len_of_str;
    return pos;
}


static FILE * preprocess_programm_from_source(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PRESPROCESSING\n");
        abort();
    }
    FILE *preprocessed_file_pointer = fopen("build/preprocessed_file.txt", "wb");
    if (preprocessed_file_pointer == NULL)
    {
        fprintf(stderr, "ERROR OF PRESPROCESSING\n");
        abort();
    }
    int code = preprocess_programm(file_pointer, preprocessed_file_pointer);
    if (code)
    {
        fprintf(stderr, "ERROR OF PRESPROCESSING\n");
        abort();
    }
    return preprocessed_file_pointer;
}

Errors_of_binary transform_asm_to_binary(FILE *file_pointer)
{
    if (file_pointer == NULL)
    {
        return ERROR_OF_READING_CMDS;
    }
    FILE *file_out_pointer = fopen("build/test.o", "wb");
    if (file_out_pointer == NULL)
    {
        return ERROR_OF_CREATING_OUT_FILE;
    }
    FILE *file_for_analyze = preprocess_programm_from_source(file_pointer);
    if (file_for_analyze == NULL)
    {
        return ERROR_OF_PREPROCESS_SOURCE_FILE;
    }
    fclose(file_for_analyze);
    file_for_analyze = fopen("build/preprocessed_file.txt", "rb");
    struct Binary_file binary_struct = {0};
    Errors_of_binary error = create_bin_data_for_binary_file(&binary_struct, file_for_analyze);
    if (error != NO_BINARY_ERRORS)
    {
        return error;
    }
    // binary_struct.buffer_of_commands = (uint8_t *) calloc (50, sizeof(uint8_t));
    // binary_struct.len_buffer_of_commands = 50;
    // if (binary_struct.buffer_of_commands == NULL)
    // {
    //     return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    // }
    // uint8_t data[50] = {0xB8, 0x2A, 0x00, 0x00, 0x00, 0xC3, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xB8, 0x3C, 0x00, 0x00, 0x00, 0x48, 0x31, 0xff, 0x0f, 0x05};
    // memcpy(binary_struct.buffer_of_commands, data, 50);
    binary_struct.file_pointer = file_out_pointer;
    // binary_struct.count_of_functions = 2;
    // binary_struct.all_functions = (struct Function_type *) calloc (binary_struct.count_of_functions, sizeof(struct Function_type));
    // if (binary_struct.all_functions == NULL)
    // {
    //     return ERROR_OF_OPERATING_CMD;
    // }
    // struct Function_type str_data[50] = {{.name = "func", .position_in_strtab = 0, .offset_in_text = 6},
    //                                      {.name = "_start", .position_in_strtab = 0, .offset_in_text = 15}};
    // for (size_t i = 0; i < binary_struct.count_of_functions; i++)
    // {
    //     strncpy((binary_struct.all_functions)[i].name, str_data[i].name, strlen(str_data[i].name));
    //     (binary_struct.all_functions)[i].position_in_strtab = str_data[i].position_in_strtab;
    //     (binary_struct.all_functions)[i].offset_in_text = str_data[i].offset_in_text; 
    // }
    error = create_binary_file(&binary_struct);
    if (error != NO_BINARY_ERRORS)
    {
        return error;
    }
    free(binary_struct.buffer_of_text_commands);
    free(binary_struct.buffer_of_data_commands);
    free(binary_struct.all_functions);
    free(binary_struct.relocation_table);
    free(binary_struct.data_labels);
    free(binary_struct.text_labels);
    fclose(file_out_pointer);
    fclose(file_for_analyze);
    return NO_BINARY_ERRORS;
}

static Errors_of_binary create_binary_file(struct Binary_file *binary_struct)
{
    if (binary_struct == NULL)
    {
        return ERROR_OF_CREATING_HEADER;
    }
    binary_struct->len_section_header_str_table = 1;
    (binary_struct->section_header_str_table)[0] = '\0';
    unsigned section_header_name_data      = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".data");
    unsigned section_header_name_text      = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".text");
    unsigned section_header_name_strtab    = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".strtab");
    unsigned section_header_name_symtab    = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".symtab");
    unsigned section_header_name_rela_text = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".rela.text");
    unsigned section_header_name_shstrtab  = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".shstrtab");
    unsigned section_header_name_gnustack  = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".note.GNU-stack");
    binary_struct->len_symbol_string_table = 1;
    (binary_struct->symbol_string_table)[0] = '\0';
    unsigned string_name_file  = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), "transform_asm_to_bin.cpp");
    for (size_t index = 0; index < binary_struct->count_of_functions; index++)
    {
        (binary_struct->all_functions)[index].position_in_strtab = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), (binary_struct->all_functions)[index].name);
    }
    for (size_t index = 0; index < binary_struct->size_of_data_labels; index++)
    {
        //printf("label_name = %s\n", (binary_struct->data_labels)[index].label_name);
        (binary_struct->data_labels)[index].position_in_strtab = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), (binary_struct->data_labels)[index].label_name);
    }
    //unsigned string_name_func  = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), "func");
    const int SYM_COUNT = 3 + binary_struct->count_of_functions + binary_struct->size_of_data_labels;
    //printf("SYM_COUNT = %d\n", SYM_COUNT);
    Elf64_Sym *syms = (Elf64_Sym *) calloc (SYM_COUNT, sizeof(Elf64_Sym));
    if (syms == NULL)
    {
        return ERROR_OF_CREATING_HEADER;
    }
    //NULL symbol
    syms[0].st_name = 0;
    syms[0].st_info = 0;
    syms[0].st_other = 0;
    syms[0].st_shndx = 0;
    syms[0].st_value = 0;
    syms[0].st_size = 0;
    // [1] file
    syms[1].st_name = string_name_file;
    syms[1].st_info = ELF64_ST_INFO(STB_LOCAL, STT_FILE);
    syms[1].st_shndx = SHN_ABS;
    // [2] section .text
    syms[2].st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION);
    // [3-...] myfunctions
    size_t offset_in_text = 0;
    for (size_t index = 3; index < SYM_COUNT - binary_struct->size_of_data_labels; index++)
    {
        if (index - 3 > binary_struct->count_of_functions)
        {
            break;
        }
        syms[index].st_name = (binary_struct->all_functions)[index - 3].position_in_strtab;
        syms[index].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
        syms[index].st_shndx = 2; // will be .text index
        syms[index].st_value = offset_in_text;
        syms[index].st_size = (Elf64_Xword)(binary_struct->all_functions)[index - 3].offset_in_text;
        offset_in_text += (binary_struct->all_functions)[index - 3].offset_in_text;
    }
    for (size_t index = 3 + binary_struct->count_of_functions, id = 0; index < SYM_COUNT && id < binary_struct->size_of_data_labels; index++, id++)
    {
        size_t index_in_rel_table = 0;
        for (index_in_rel_table = 0; index_in_rel_table < (binary_struct->size_of_relocation_table); index_in_rel_table++)
        {
            if (strcasecmp((binary_struct->data_labels)[id].label_name, (binary_struct->relocation_table)[index_in_rel_table].symbol_name) == 0)
            {
                (binary_struct->relocation_table)[index_in_rel_table].symbol_index = index;
            }
        }
        syms[index].st_name = (binary_struct->data_labels)[id].position_in_strtab;
        syms[index].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE);
        if ((binary_struct->data_labels)[id].type == CONSTANT_LABEL)
        {
            syms[index].st_shndx = SHN_ABS;
        }
        else
        {
            syms[index].st_shndx = 1;
        }
        syms[index].st_value = (binary_struct->data_labels)[id].pc;
        syms[index].st_size = (Elf64_Xword)(binary_struct->data_labels)[id].size_of_data;
        //printf("(binary_struct->size_of_relocation_table) = %lu\n", (binary_struct->size_of_relocation_table));
        if (index_in_rel_table == (binary_struct->size_of_relocation_table))
        {
            continue;
        }
        //(binary_struct->relocation_table)[index_in_rel_table].symbol_index = index;
    }
    // syms[3].st_name = string_name_func;
    // syms[3].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
    // syms[3].st_shndx = 1;    // will be .text index (we place .text as sec idx 1)
    // syms[3].st_value = 0;    // offset in .text
    // syms[3].st_size  = (Elf64_Xword)(binary_struct->len_buffer_of_commands);
    const int LOCAL_COUNT = 3; // symbols 0..2 are local -> first non-local has index 3
    // layout sections in file (indices: 0=NULL,1=.data,2=.text,3=.rela.text,4=.shstrtab,5=.symtab,6=.strtab,7=.note.GNU-stack)
    const int SHNUM = 8;
    Elf64_Shdr sh[SHNUM] = {0};
    size_t offset = sizeof(Elf64_Ehdr);
    // .data
    offset = align_up(offset, 8);
    size_t data_offset = offset;
    //printf("data_offset = %lu\n", data_offset);
    //printf("len_buffer_of_data_commands = %lu\n", binary_struct->len_buffer_of_data_commands);
    offset += binary_struct->len_buffer_of_data_commands;
    // .text
    offset = align_up(offset, 16); 
    size_t text_offset = offset;
    //printf("text_offset = %lu\n", text_offset);
    offset += binary_struct->len_buffer_of_text_commands;
    //printf("offset = %lu\n", offset);
    //printf("offset = %lu\n", offset);
    // .rela.text
    offset = align_up(offset, 8);
    size_t rela_text_offset = offset;
    offset += binary_struct->size_of_relocation_table * sizeof(Elf64_Rela);
    // .shstrtab
    size_t base = next_power_of_two(binary_struct->len_section_header_str_table);
    offset = align_up(offset, base); 
    //printf("len_of_section_header_str_table = %lu\n", binary_struct->len_section_header_str_table);
    size_t shstr_offset = offset; 
    offset += binary_struct->len_section_header_str_table;
    // .strtab
    offset = align_up(offset, 1); 
    size_t strtab_offset = offset; 
    offset += binary_struct->len_symbol_string_table;
    // .symtab
    offset = align_up(offset, 8); 
    size_t symtab_offset = offset; 
    size_t symtab_size = SYM_COUNT * sizeof(Elf64_Sym); 
    offset += symtab_size;
    // .note.GNU-stack (empty)
    offset = align_up(offset, 1); 
    size_t gnustack_offset = offset; 
    size_t gnustack_size = 0;
    // section header table
    offset = align_up(offset, 8); 
    size_t section_header_offset = offset; 
    size_t section_header_table_size = SHNUM * sizeof(Elf64_Shdr); 
    offset += section_header_table_size;

    const int INDEX_NULL = 0;
    const int INDEX_DATA = 1;
    const int INDEX_TEXT = 2;
    const int INDEX_SHSRTAB = 6;
    const int INDEX_STRTAB = 3;
    const int INDEX_SYMTAB = 4;
    const int INDEX_RELA_TEXT = 5;
    const int INDEX_GNU_STACK = 7;

    //.data
    sh[INDEX_DATA].sh_name = section_header_name_data;
    sh[INDEX_DATA].sh_type = SHT_PROGBITS;
    sh[INDEX_DATA].sh_flags = SHF_ALLOC | SHF_WRITE;
    sh[INDEX_DATA].sh_offset = data_offset;
    sh[INDEX_DATA].sh_size = binary_struct->len_buffer_of_data_commands;
    sh[INDEX_DATA].sh_addralign = 8;

    //.text
    sh[INDEX_TEXT].sh_name = section_header_name_text;
    sh[INDEX_TEXT].sh_type = SHT_PROGBITS;
    sh[INDEX_TEXT].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    sh[INDEX_TEXT].sh_offset = text_offset;
    sh[INDEX_TEXT].sh_size = binary_struct->len_buffer_of_text_commands;
    //printf("len_of_buffer_text_commands = %lu\n", binary_struct->len_buffer_of_text_commands);
    sh[INDEX_TEXT].sh_addralign = 16;


    //.shstrtab
    sh[INDEX_SHSRTAB].sh_name = section_header_name_shstrtab;
    sh[INDEX_SHSRTAB].sh_type = SHT_STRTAB;
    sh[INDEX_SHSRTAB].sh_offset = shstr_offset;
    sh[INDEX_SHSRTAB].sh_size = (binary_struct->len_section_header_str_table);
    sh[INDEX_SHSRTAB].sh_addralign = 1;

    //.strtab
    sh[INDEX_STRTAB].sh_name = section_header_name_strtab;
    sh[INDEX_STRTAB].sh_type = SHT_STRTAB;
    sh[INDEX_STRTAB].sh_offset = strtab_offset;
    sh[INDEX_STRTAB].sh_size = binary_struct->len_symbol_string_table;
    sh[INDEX_STRTAB].sh_addralign = 1;

    //.symtab
    sh[INDEX_SYMTAB].sh_name = section_header_name_symtab;
    sh[INDEX_SYMTAB].sh_type = SHT_SYMTAB;
    sh[INDEX_SYMTAB].sh_offset = symtab_offset;
    sh[INDEX_SYMTAB].sh_size = symtab_size;
    sh[INDEX_SYMTAB].sh_link = INDEX_STRTAB; // .strtab index
    sh[INDEX_SYMTAB].sh_info = LOCAL_COUNT;
    sh[INDEX_SYMTAB].sh_addralign = 8;
    sh[INDEX_SYMTAB].sh_entsize = sizeof(Elf64_Sym);

    //.rela.text
    sh[INDEX_RELA_TEXT].sh_name = section_header_name_rela_text;
    sh[INDEX_RELA_TEXT].sh_type = SHT_RELA;
    sh[INDEX_RELA_TEXT].sh_entsize = sizeof(Elf64_Rela);
    sh[INDEX_RELA_TEXT].sh_flags = 0;
    sh[INDEX_RELA_TEXT].sh_offset = rela_text_offset;
    sh[INDEX_RELA_TEXT].sh_size = binary_struct->size_of_relocation_table * sizeof(Elf64_Rela);
    sh[INDEX_RELA_TEXT].sh_link = INDEX_SYMTAB;  // index .symtab
    sh[INDEX_RELA_TEXT].sh_info = INDEX_TEXT;  // index .text
    sh[INDEX_RELA_TEXT].sh_addralign = 8;

    //.note.GNU-stack(empty)
    sh[INDEX_GNU_STACK].sh_name = section_header_name_gnustack;
    sh[INDEX_GNU_STACK].sh_type = SHT_PROGBITS;
    sh[INDEX_GNU_STACK].sh_offset = gnustack_offset;
    sh[INDEX_GNU_STACK].sh_size = gnustack_size;
    sh[INDEX_GNU_STACK].sh_addralign = 1;

    memset(&(binary_struct->ELF_Header), 0, sizeof(binary_struct->ELF_Header));
    (binary_struct->ELF_Header).e_ident[EI_MAG0] = ELFMAG0; 
    (binary_struct->ELF_Header).e_ident[EI_MAG1] = ELFMAG1; 
    (binary_struct->ELF_Header).e_ident[EI_MAG2] = ELFMAG2; 
    (binary_struct->ELF_Header).e_ident[EI_MAG3] = ELFMAG3;
    (binary_struct->ELF_Header).e_ident[EI_CLASS] = ELFCLASS64;
    (binary_struct->ELF_Header).e_ident[EI_DATA] = ELFDATA2LSB;
    (binary_struct->ELF_Header).e_ident[EI_VERSION] = EV_CURRENT;
    (binary_struct->ELF_Header).e_type = ET_REL;
    (binary_struct->ELF_Header).e_machine = EM_X86_64;
    (binary_struct->ELF_Header).e_version = EV_CURRENT;
    (binary_struct->ELF_Header).e_ehsize = sizeof(Elf64_Ehdr);
    (binary_struct->ELF_Header).e_shentsize = sizeof(Elf64_Shdr);
    (binary_struct->ELF_Header).e_shnum = SHNUM;
    (binary_struct->ELF_Header).e_shoff = section_header_offset;
    (binary_struct->ELF_Header).e_shstrndx = INDEX_SHSRTAB; // index .shstrtab

    // printf("e_shstrndx = %u\n", binary_struct->ELF_Header.e_shstrndx);
    // for (int i = 0; i < SHNUM; i++) 
    // {
    //     printf("sh[%d].sh_name = %u -> \"%s\"\n", i,
    //         sh[i].sh_name,
    //         binary_struct->section_header_str_table + sh[i].sh_name);
    // }

    // for (size_t index = 0; index < (binary_struct->len_section_header_str_table); index++)
    // {
    //     printf("%c", (binary_struct->section_header_str_table)[index]);
    //     if ((binary_struct->section_header_str_table)[index] == '\0')
    //     {
    //         printf("\n");
    //     }
    // }
    // printf("\n");


    fwrite(&(binary_struct->ELF_Header), 1, sizeof(binary_struct->ELF_Header), binary_struct->file_pointer);
    //shstrtab
    fseek(binary_struct->file_pointer, (long)shstr_offset, SEEK_SET); 
    fwrite(binary_struct->section_header_str_table, 1, binary_struct->len_section_header_str_table, binary_struct->file_pointer);

    //.data
    fseek(binary_struct->file_pointer, (long)data_offset, SEEK_SET); 
    fwrite(binary_struct->buffer_of_data_commands, 1, binary_struct->len_buffer_of_data_commands, binary_struct->file_pointer);
    //.text
    fseek(binary_struct->file_pointer, (long)text_offset, SEEK_SET);
    fwrite(binary_struct->buffer_of_text_commands, 1, binary_struct->len_buffer_of_text_commands, binary_struct->file_pointer);
    // //shstrtab
    // fseek(binary_struct->file_pointer, (long)shstr_offset, SEEK_SET); 
    // fwrite(binary_struct->section_header_str_table, 1, binary_struct->len_section_header_str_table, binary_struct->file_pointer);
    //strtab
    fseek(binary_struct->file_pointer, (long)strtab_offset, SEEK_SET); 
    fwrite(binary_struct->symbol_string_table, 1, binary_struct->len_symbol_string_table, binary_struct->file_pointer);
    //symtab
    fseek(binary_struct->file_pointer, (long)symtab_offset, SEEK_SET);
    fwrite(syms, 1, symtab_size, binary_struct->file_pointer);
    //.rela.text
    fseek(binary_struct->file_pointer, (long)rela_text_offset, SEEK_SET);
    for (size_t index = 0; index < binary_struct->size_of_relocation_table; index++)
    {
        Elf64_Rela rela = {0};
        rela.r_offset = (Elf64_Addr)(binary_struct->relocation_table)[index].offset_in_text;
        //printf("sym.st_name[%lu] = %u\n", (binary_struct->relocation_table)[index].symbol_index, syms[(binary_struct->relocation_table)[index].symbol_index].st_name);
        //printf("symbol_name = %s\nsymbol_index = %d\n", (binary_struct->relocation_table)[index].symbol_name, (binary_struct->relocation_table)[index].symbol_index);
        rela.r_info = ELF64_R_INFO((binary_struct->relocation_table)[index].symbol_index, (binary_struct->relocation_table)[index].type);
        rela.r_addend = (Elf64_Sxword)(binary_struct->relocation_table)[index].additional_offset;
        fwrite(&rela, 1, sizeof(rela), binary_struct->file_pointer);
    }
    
    // .note.GNU-stack empty
    fseek(binary_struct->file_pointer, (long)section_header_offset, SEEK_SET); 
    fwrite(sh, 1, section_header_table_size, binary_struct->file_pointer);
    free(syms);
    return NO_BINARY_ERRORS;
}

static Register get_register(const char *register_name)
{
    if (strcasecmp(register_name, "rax") == 0)
    {
        return RAX;
    }
    if (strcasecmp(register_name, "rbx") == 0)
    {
        return RBX;
    }
    if (strcasecmp(register_name, "rcx") == 0)
    {
        return RCX;
    }
    if (strcasecmp(register_name, "rdx") == 0)
    {
        return RDX;
    }
    if (strcasecmp(register_name, "rsp") == 0)
    {
        return RSP;
    }
    if (strcasecmp(register_name, "rsi") == 0)
    {
        return RSI;
    }
    if (strcasecmp(register_name, "rdi") == 0)
    {
        return RDI;
    }
    if (strcasecmp(register_name, "rbp") == 0)
    {
        return RBP;
    }
    if (strcasecmp(register_name, "r8") == 0)
    {
        return R8;
    }
    if (strcasecmp(register_name, "r9") == 0)
    {
        return R9;
    }
    if (strcasecmp(register_name, "r10") == 0)
    {
        return R10;
    }
    if (strcasecmp(register_name, "r11") == 0)
    {
        return R11;
    }
    if (strcasecmp(register_name, "r12") == 0)
    {
        return R12;
    }
    if (strcasecmp(register_name, "r13") == 0)
    {
        return R13;
    }
    if (strcasecmp(register_name, "r14") == 0)
    {
        return R14;
    }
    if (strcasecmp(register_name, "r15") == 0)
    {
        return R15;
    }

    // if (strcasecmp(register_name, "eax") == 0)
    // {
    //     return EAX;
    // }
    // if (strcasecmp(register_name, "ebx") == 0)
    // {
    //     return EBX;
    // }
    // if (strcasecmp(register_name, "ecx") == 0)
    // {
    //     return ECX;
    // }
    // if (strcasecmp(register_name, "edx") == 0)
    // {
    //     return EDX;
    // }
    return UNKNOWN_REGISTER;
}

static Initial_instructions get_initial_instruction(const char *instruction)
{
    if (strcasecmp(instruction, "db") == 0)
    {
        return INSTRUCTION_DB;
    }
    if (strcasecmp(instruction, "dw") == 0)
    {
        return INSTRUCTION_DW;
    }
    if (strcasecmp(instruction, "dd") == 0)
    {
        return INSTRUCTION_DD;
    }
    if (strcasecmp(instruction, "dq") == 0)
    {
        return INSTRUCTION_DQ;
    }
    return UNKNOWN_INSTRUCTION;
}

static void add_label(struct Data_CMDS *commands, const char *label_name, int32_t pc, Sections section, size_t size_of_data, Type_of_label type, int64_t imm_data)
{
    if (commands == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m There is no data commands\n");
        abort();
    }
    if (section == SECTION_DATA)
    {
        for (size_t index = 0; index < (commands->size_of_labels_data); index++)
        {
            if (strcasecmp(((commands->labels_data)[index]).label_name, label_name) == 0)
            {
                fprintf(stderr, "Dublicate label: %s\n", label_name);
                abort();
            }
        }
        strncpy(((commands->labels_data)[commands->size_of_labels_data]).label_name, label_name, strlen(label_name));
        ((commands->labels_data)[commands->size_of_labels_data]).pc = pc;
        ((commands->labels_data)[commands->size_of_labels_data]).size_of_data = size_of_data;
        ((commands->labels_data)[commands->size_of_labels_data]).type = type;
        ((commands->labels_data)[commands->size_of_labels_data]).imm_data = imm_data;
        (commands->size_of_labels_data)++;
        return;
    }
    for (size_t index = 0; index < (commands->size_of_labels_text); index++)
    {
        if (strcasecmp(((commands->labels_text)[index]).label_name, label_name) == 0)
        {
            fprintf(stderr, "Dublicate label: %s\n", label_name);
            abort();
        }
    }
    strncpy(((commands->labels_text)[commands->size_of_labels_text]).label_name, label_name, strlen(label_name));
    ((commands->labels_text)[commands->size_of_labels_text]).pc = pc;
    ((commands->labels_text)[commands->size_of_labels_text]).size_of_data = size_of_data;
    ((commands->labels_data)[commands->size_of_labels_data]).type = type;
    (commands->size_of_labels_text)++;
    return;
}

static int find_label_and_change_it(struct Data_CMDS *commands, const char *label_name, size_t size_of_data, Sections section, Type_of_label type, int64_t imm_data)
{
    if (commands == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m There is no data commands\n");
        abort();
    }
    if (section == SECTION_TEXT)
    {
        for (size_t index = 0; index < (commands->size_of_labels_text); index++)
        {
            if (strcasecmp((commands->labels_text)[index].label_name, label_name) == 0)
            {
                (commands->labels_text)[index].type = type;
                (commands->labels_text)[index].size_of_data = size_of_data;
                (commands->labels_text)[index].imm_data = imm_data;
                //(commands->labels_text)[index].pc += size_of_data;
                return ((commands->labels_text)[index].pc);
            }
        }
    }
    else if (section == SECTION_DATA)
    {
        for (size_t index = 0; index < (commands->size_of_labels_data); index++)
        {
            if (strcasecmp((commands->labels_data)[index].label_name, label_name) == 0)
            {
                (commands->labels_data)[index].type = type;
                (commands->labels_data)[index].size_of_data = size_of_data;
                (commands->labels_data)[index].imm_data = imm_data;
                //(commands->labels_data)[index].pc += size_of_data;
                return ((commands->labels_data)[index].pc);
            }
        }
    }
    return -1;
}

static struct Label get_label(struct Data_CMDS *commands, const char *label_name, Sections section)
{
    if (commands == NULL || label_name == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m There is no commands or label_name\n");
        abort();
    }
    if (section == SECTION_TEXT)
    {
        for (size_t index = 0; index < (commands->size_of_labels_text); index++)
        {
            if (strcasecmp((commands->labels_text)[index].label_name, label_name) == 0)
            {
                return ((commands->labels_text)[index]);
            }
        }
    }
    else if (section == SECTION_DATA)
    {
        for (size_t index = 0; index < (commands->size_of_labels_data); index++)
        {
            if (strcasecmp((commands->labels_data)[index].label_name, label_name) == 0)
            {
                return ((commands->labels_data)[index]);
            }
        }
    }
    return {};
}

static void parse_section_text(struct Binary_file *binary_struct, struct Data_CMDS *commands, FILE *file_pointer_with_commands, size_t size_of_file)
{
    if (binary_struct == NULL || commands == NULL || file_pointer_with_commands == NULL)
    {
        fprintf(stderr, "Error! Error of parsing section text\n");
        abort();
    }
    int32_t text_pc = 0;
    char str_for_input[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    char *end_pointer = (str_for_input + MAX_LEN_FOR_STATIC_ARRAYS);
    Sections section = SECTION_TEXT;
    int index_of_last_function = -1;
    while(fgets(str_for_input, sizeof(str_for_input), file_pointer_with_commands))
    {
        struct Instruction instruction = {};
        char *start = str_for_input;
        start = skip_spaces(start, end_pointer);
        if (*start == '\0' || *start == ';')
        {
            continue;
        }
        start = skip_spaces(start, end_pointer);
        char label_name[MAX_LEN_FOR_STATIC_ARRAYS] = "";
        char new_label_name[MAX_LEN_FOR_STATIC_ARRAYS] = "";
        char *old_start = start;
        if (sscanf(start, "%s", label_name) == 1)
        {
            size_t len_of_label = strlen(label_name);
            start += len_of_label;
            bool is_label = false;
            for (size_t id = 0; id < len_of_label; id++)
            {
                if (label_name[id] == ':')
                {
                    is_label = true;
                    break;
                }
            }
            snprintf(new_label_name, len_of_label, "%s", label_name);
            if (is_label)
            {
                bool result = check_if_function(binary_struct, new_label_name);
                if (!result)
                {
                    add_label(commands, new_label_name, text_pc, section, 0, LABEL_FOR_JMP, 0);
                    continue;
                }
                if (index_of_last_function != -1)
                {
                    (binary_struct->all_functions)[index_of_last_function].offset_in_text = text_pc;
                }
                for (size_t index = 0; index < (binary_struct->count_of_functions); index++)
                {
                    if (strcasecmp((binary_struct->all_functions)[index].name, new_label_name) == 0)
                    {
                        index_of_last_function = index;
                        break;
                    }
                }   
            }
            else
            {
                start = old_start;
            }
            instruction.opcode = UNKNOWN_OPCODE;
            instruction.section = section;
            //printf("start = %s\n\n", start);
            start = skip_spaces(start, end_pointer);
            parse_instruction_from_text(start, &instruction, end_pointer, &text_pc, commands);
            //printf("text_pc = %u\n", text_pc);
        }
        else
        {
            fprintf(stderr, "Error! Error of processing string at section .text\n");
            abort();
        }
        commands->instructions[commands->size_of_instructions++] = instruction;
        if (commands->size_of_instructions >= size_of_file)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Not enough memory in \x1b[33mcommands.instructions\x1b[0m\n");
            abort();
        }
    }
    (binary_struct->all_functions)[index_of_last_function].offset_in_text = text_pc;
    return;
}

static void parse_section_data(struct Binary_file *binary_struct, struct Data_CMDS *commands, FILE *file_pointer_with_commands, size_t size_of_file)
{
    if (binary_struct == NULL || commands == NULL || file_pointer_with_commands == NULL)
    {
        fprintf(stderr, "Error! Error of parsing section data\n");
        abort();
    }
    int32_t data_pc = 0;
    char str_for_input[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    char *end_pointer = (str_for_input + MAX_LEN_FOR_STATIC_ARRAYS);
    Sections section = SECTION_DATA;
    while(fgets(str_for_input, sizeof(str_for_input), file_pointer_with_commands))
    {
        struct Instruction instruction = {};
        char *start = str_for_input;
        start = skip_spaces(start, end_pointer);
        if (*start == '\0' || *start == ';')
        {
            continue;
        }
        if (strcasecmp(str_for_input, "section .data\n") == 0)
        {
            continue;
        }
        if (strcasecmp(str_for_input, "section .text\n") == 0)
        {
            break;
        }
        start = skip_spaces(start, end_pointer);
        char label_name[MAX_LEN_FOR_STATIC_ARRAYS] = "";
        if (sscanf(start, "%s", label_name) == 1)
        {
            size_t len_of_label = strlen(label_name);
            start += len_of_label;
            char operation[MAX_LEN_FOR_STATIC_ARRAYS] = "";
            char *old_start = start;
            if (sscanf(start, "%s", operation) ==  1)
            {
                if (strcasecmp(operation, "equ") == 0)
                {
                    char label_in_expression[MAX_LEN_FOR_STATIC_ARRAYS] = "";
                    start += strlen(operation) + 1;
                    start = skip_spaces(start, end_pointer);
                    sscanf(start, "%s", label_in_expression);
                    if (strcasecmp(label_in_expression, "$") == 0)
                    {
                        start = skip_spaces(start, end_pointer);
                        start++;
                        start = skip_spaces(start, end_pointer);
                        start++;
                        sscanf(start, "%s", label_in_expression);
                        struct Label label = get_label(commands, label_in_expression, SECTION_DATA);
                        //printf("label_data_in_equ = %lu\n", label.imm_data);
                        add_label(commands, label_name, data_pc, SECTION_DATA, label.size_of_data, CONSTANT_LABEL, label.imm_data);
                        //data_pc += label.imm_data;
                        continue;
                    }
                }
                else
                {
                    start = old_start;
                }
            }
            else
            {
                fprintf(stderr, "Error! Error of processing string at section .data\n");
                abort();
            }
            //printf("label_name = %s\n", label_name);
            add_label(commands, label_name, data_pc, section, 0, LABEL_FOR_JMP, 0);

            // for (size_t id = 0; id < commands->size_of_labels_data; ++id)
            // {
            //     printf("label_name = %s\n", (commands->labels_data)[id].label_name);
            // }
            instruction.opcode = UNKNOWN_OPCODE;
            instruction.section = section;
            //printf("start = %s\n\n", start);
            start = skip_spaces(start, end_pointer);
            strcpy((instruction.label).label_name, label_name);
            parse_instruction_from_data(start, &instruction, end_pointer, &data_pc, commands);
            //printf("data_pc = %u\n", data_pc);
        }
        else
        {
            fprintf(stderr, "Error! Error of processing string at section .data\n");
            abort();
        }
        //printf("len_of_operand in parse_section_data = %lu\n", instruction.operands.len_of_string_operand);
        commands->instructions[commands->size_of_instructions++] = instruction;
        if (commands->size_of_instructions >= size_of_file)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Not enough memory in \x1b[33mcommands.instructions\x1b[0m\n");
            abort();
        }

    }
    return;
}




static Errors_of_binary create_bin_data_for_binary_file(struct Binary_file *binary_struct, FILE *file_pointer_with_commands)
{
    if (binary_struct == NULL || file_pointer_with_commands == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    size_t size_of_file = get_size_of_file(file_pointer_with_commands);
    // int32_t text_pc = 0;//programm counter in text
    // int32_t data_pc = 0;
    struct Data_CMDS commands = {};
    commands.instructions = (struct Instruction *) calloc (size_of_file, sizeof(struct Instruction));
    if (commands.instructions == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    commands.labels_text = (struct Label *) calloc (size_of_file, sizeof(struct Label));
    if (commands.labels_text == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    commands.labels_data = (struct Label *) calloc (size_of_file, sizeof(struct Label));
    if (commands.labels_data == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    commands.size_of_instructions = 0;
    commands.size_of_labels_text = 0;
    commands.size_of_labels_data = 0;
    get_all_functions(binary_struct, file_pointer_with_commands);
    parse_section_data(binary_struct, &commands, file_pointer_with_commands, size_of_file);
    parse_section_text(binary_struct, &commands, file_pointer_with_commands, size_of_file);
    binary_struct->buffer_of_text_commands = (uint8_t *) calloc (size_of_file, sizeof(uint8_t));
    if (binary_struct->buffer_of_text_commands == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    binary_struct->len_buffer_of_text_commands = 0;
    binary_struct->buffer_of_data_commands = (uint8_t *) calloc (size_of_file, sizeof(uint8_t));
    if (binary_struct->buffer_of_data_commands == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    binary_struct->len_buffer_of_data_commands = 0;
    encode_data_instructions(&commands, binary_struct);
    encode_text_instructions(&commands, binary_struct);
    
    // for (size_t index = 0; index <commands.size_of_labels_data; index++)
    // {
    //     printf("label = %s\n", (commands.labels_data)[index].label_name);
    // }
    free(commands.instructions);
    free(commands.labels_text);
    free(commands.labels_data);
    return NO_BINARY_ERRORS;
}

static void parse_instruction_from_text(char *position, struct Instruction *instruction, char *end_pointer, int32_t *pc, struct Data_CMDS *commands)
{
    if (position == NULL || instruction == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0mIt is impossible to parse instruction from text\n");
        abort();
    }
    char operation[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    if (sscanf(position, "%s", operation) != 1)
    {
        return;
    }
    //printf("operation = %s\n", operation);
    position += strlen(operation);
    if (strcasecmp(operation, "mov") == 0)
    {
        char param_1[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        char param_2[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        position = skip_spaces(position, end_pointer);
        int res = sscanf(position, "%7[^,]", param_1);
        position += strlen(param_1);
        position += 2;
        res += sscanf(position, "%s", param_2);
        //printf("param_1 = %s\nparam_2 = %s\n", param_1, param_2);
        if (res != 2)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Error in parsing mov\n");
            abort();
        }
        bool is_addr = false;
        if (param_2[0] == '[')
        {
            is_addr = true;
            size_t index = 0;
            char new_param_2[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
            while (index < STATIC_LEN_FOR_SMALL_ARRAYS && isspace(param_2[index]))
            {
                index++;
            }
            if (index + 2 < STATIC_LEN_FOR_SMALL_ARRAYS && param_2[index] == 'r' && param_2[index + 1] == 'e' && param_2[index + 2] == 'l')
            {
                printf("it is relative offset! WIP\n");
                abort();
            }
            else
            {
                index = 1;
                while (index < STATIC_LEN_FOR_SMALL_ARRAYS && param_2[index] != ']')
                {
                    new_param_2[index - 1] = param_2[index];
                    index++;
                }
                memset(param_2, '\0', STATIC_LEN_FOR_SMALL_ARRAYS);
                strncpy(param_2, new_param_2, STATIC_LEN_FOR_SMALL_ARRAYS);
            }
        }
        if (!is_addr)
        {
            bool is_digit = is_str_digit(param_2);
            int64_t imm = 0;
            if (is_digit)
            {
                char *end = NULL;
                imm = strtol(param_2, &end, 10);
            }
            instruction->opcode = OP_MOV_REG_REG;
            instruction->register_dest = get_register(param_1);
            if (is_digit)
            {
                instruction->opcode = OP_MOV_REG_IMM;
                instruction->imm = imm;
                (*pc) += 1/*opcode*/ + 8/*imm64*/ + 1/*REX*/;
            }
            else
            {
                instruction->register_source = get_register(param_2);
                if (instruction->register_source == UNKNOWN_REGISTER)
                {
                    struct Label label = get_label(commands, param_2, SECTION_DATA);
                    
                    if (label.type == CONSTANT_LABEL)
                    {
                        instruction->opcode = OP_MOV_REG_IMM;
                        instruction->imm = label.imm_data;
                    }
                    else
                    {
                        instruction->opcode = OP_MOV_REG_LABEL;
                        strcpy((instruction->label).label_name, param_2);
                    }
                }
                (*pc) += 1/*opcode*/ + 1/*ModR/M*/ + 1/*REX*/;
            }
        }
        else
        {
            instruction->opcode = OP_MOV_REG_ADDRESS_REG;
            instruction->register_dest = get_register(param_1);
            instruction->register_source = get_register(param_2);
            if (instruction->register_source == UNKNOWN_REGISTER)
            {
                struct Label label = get_label(commands, param_2, SECTION_DATA);
                instruction->opcode = OP_MOV_REG_ADDRESS_LABEL;
                strcpy((instruction->label).label_name, param_2);
            }
            (*pc) += 1/*opcode*/ + 1/*ModR/M*/ + 1/*REX*/;
        }
    }
    else if (strcasecmp(operation, "lea") == 0)
    {
        char param_1[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        char param_2[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        position = skip_spaces(position, end_pointer);
        int res = sscanf(position, "%7[^,]", param_1);
        position += strlen(param_1);
        position += 2;
        res += sscanf(position, "%s", param_2);
        if (param_2[0] == '[')
        {
            size_t index = 0;
            char new_param_2[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
            while (index < STATIC_LEN_FOR_SMALL_ARRAYS && isspace(param_2[index]))
            {
                index++;
            }
            if (index + 2 < STATIC_LEN_FOR_SMALL_ARRAYS && param_2[index] == 'r' && param_2[index + 1] == 'e' && param_2[index + 2] == 'l')
            {
                printf("it is relative offset! WIP\n");
                abort();
            }
            else
            {
                index = 1;
                while (index < STATIC_LEN_FOR_SMALL_ARRAYS && param_2[index] != ']')
                {
                    new_param_2[index - 1] = param_2[index];
                    index++;
                }
                memset(param_2, '\0', STATIC_LEN_FOR_SMALL_ARRAYS);
                strncpy(param_2, new_param_2, STATIC_LEN_FOR_SMALL_ARRAYS);
            }
        }
        //printf("param_1 = %s\nparam_2 = %s\n", param_1, param_2);
        if (res != 2)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Error in parsing mov\n");
            abort();
        }
        instruction->opcode = OP_LEA_REG_ABS_ADDRESS_REG;
        instruction->register_dest = get_register(param_1);
        //FIX IT(recursive descent!!!)
        instruction->register_source = get_register(param_2);
        if (instruction->register_source == UNKNOWN_REGISTER)
        {
            struct Label label = get_label(commands, param_2, SECTION_DATA);
            instruction->opcode = OP_LEA_REG_ABS_ADDRESS_LABEL;
            strcpy((instruction->label).label_name, param_2);
        }
        (*pc) += 1/*opcode*/ + 1/*ModR/M*/ + 1/*REX*/;
    }
    else if (strcasecmp(operation, "xor") == 0)
    {
        char param_1[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        char param_2[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        position = skip_spaces(position, end_pointer);
        int res = sscanf(position, "%7[^,]", param_1);
        position += strlen(param_1);
        position += 2;
        res += sscanf(position, "%s", param_2);
        //printf("param_1 = %s\nparam_2 = %s\n", param_1, param_2);
        if (res != 2)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Error in parsing mov\n");
            abort();
        }
        // bool is_digit = is_str_digit(param_2);
        // int64_t imm = 0;
        // if (is_digit)
        // {
        //     char *end = NULL;
        //     imm = strtol(param_2, &end, 10);
        // }
        instruction->opcode = OP_XOR_REG_REG;
        instruction->register_dest = get_register(param_1);
        // if (is_digit)
        // {
        //     instruction.opcode = OP_XOR_REG_IMM;
        //     instruction.imm = imm;
        //     pc += 1/*opcode*/ + 8/*imm64*/ + 1/*REX*/;
        // }
        // else
        // {
        instruction->register_source = get_register(param_2);
        (*pc) += 1/*opcode*/ + 1/*ModR/M*/ + 1/*REX*/;
        //}
    }
    else if (strcasecmp(operation, "push") == 0)
    {
        char param_1[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        position = skip_spaces(position, end_pointer);
        int res = sscanf(position, "%s", param_1);
        if (res != 1)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Error in parsing push\n");
            abort();
        }
        //printf("param_1 = %s\nlen_of_param = %lu\n", param_1, strlen(param_1));
        instruction->opcode = OP_PUSH_REG;
        instruction->register_source = get_register(param_1);
        (*pc) += 1/*opcode*/ + 1/*ModR/M*/;
    }
    else if (strcasecmp(operation, "pop") == 0)
    {
        char param_1[STATIC_LEN_FOR_SMALL_ARRAYS] = "";
        position = skip_spaces(position, end_pointer);
        int res = sscanf(position, "%s", param_1);
        if (res != 1)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Error in parsing push\n");
            abort();
        }
        instruction->opcode = OP_POP_TO_REG;
        instruction->register_dest = get_register(param_1);
        (*pc) += 1/*opcode*/ + 1/*ModR/M*/;
    }
    else if (strcasecmp(operation, "syscall") == 0)
    {
        //printf("HEREEE\n");
        instruction->opcode = OP_SYSCALL;
    }
    //printf("operation = %s\nopcode = %d\n", operation, instruction->opcode);
    instruction->pc = *pc;
    return;
}

static void parse_instruction_from_data(char *position, struct Instruction *instruction, char *end_pointer, int32_t *pc, struct Data_CMDS *commands)
{
    if (position == NULL || instruction == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0mIt is impossible to parse instruction from data\n");
        abort();
    }
    char initial_instruction[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    if (sscanf(position, "%s", initial_instruction) != 1)
    {
        return;
    }
    position += strlen(initial_instruction);
    //printf("initial_instruction = %s\n", initial_instruction);
    char list_with_operands[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    size_t id = 0;
    size_t offset = 0;
    while (*position != '\n')
    {
        list_with_operands[id] = *position;
        id++;
        position++;
    }
    //printf("list_with_operands = %s\n", list_with_operands);
    instruction->initial_instruction = get_initial_instruction(initial_instruction);
    if (instruction->initial_instruction == UNKNOWN_INSTRUCTION)
    {
        fprintf(stderr, "\x1b[33mError!\x1b[0mThere is no initial instruction in section data after label with data\n");
        abort();
    }
    bool is_number = is_str_digit(list_with_operands);
    if (is_number)
    {
        int64_t imm = 0;
        char *end = NULL;
        imm = strtol(list_with_operands, &end, 10);
        (instruction->operands).numeric_operand = imm;
        (instruction->operands).type = TYPE_NUMBER;
        *pc = find_label_and_change_it(commands, (instruction->label).label_name, (size_t)instruction->initial_instruction, SECTION_DATA, LABEL_WITH_DATA, imm);
        //*pc += (size_t)instruction->initial_instruction;
        offset = (size_t)instruction->initial_instruction;
    }
    else
    {
        //printf("list_with_operands = %s\n", list_with_operands);
        uint8_t operand[MAX_LEN_FOR_STATIC_ARRAYS] = {0};
        size_t len_of_operand = 0;
        size_t len = strlen(list_with_operands);
        bool is_between_quotation_marks = false;
        bool is_any_quotation_marks = false;
        int byte_for_dup = 0;
        if (strstr(list_with_operands, "dup"))
        {
            int count = 0;
            int value = 0;
            sscanf(list_with_operands, "%d dup(%d)", &count, &value);
            uint8_t b = value & 0xFF;
            for (int i = 0; i < count; i++) 
            {
                operand[len_of_operand++] = b;
            }
        }
        else
        {
            for (size_t index = 0; index < len; index++)
            {
                if (list_with_operands[index] == '\'' && !is_between_quotation_marks)
                {
                    is_any_quotation_marks = true;
                    is_between_quotation_marks = true;
                    continue;
                }
                else if (list_with_operands[index] == '\'' && is_between_quotation_marks)
                {
                    is_between_quotation_marks = false;
                    continue;
                }
                if ((list_with_operands[index] == ',' ||
                    list_with_operands[index] == ' ' ) && !is_between_quotation_marks)
                {   
                    continue;
                }
                if (is_between_quotation_marks)
                {
                    operand[len_of_operand++] = list_with_operands[index];
                }
                else if (isdigit(list_with_operands[index]))
                {
                    //printf("list_with_operands[%lu] = %c\n", index, list_with_operands[index]);
                    char num[MAX_LEN_FOR_STATIC_ARRAYS] = "";
                    num[0] = list_with_operands[index];
                    index++;
                    size_t num_id = 1;
                    for (; index < len && isdigit(list_with_operands[index]); ++index)
                    {
                        num[num_id++] = list_with_operands[index];
                    }
                    int number = atoi(num);
                    //printf("number = %d\n", number);
                    char new_symbol = (char)(number);
                    //printf("new_symbol = %d\n", new_symbol);
                    operand[len_of_operand++] = new_symbol;
                }
            }
        }
        //printf("operand = %s\n", operand);
        //size_t len_of_operand = strlen(operand);
        //printf("len_of_operand = %lu\n", len_of_operand);
        (instruction->operands).len_of_string_operand = len_of_operand;
        memcpy((instruction->operands).string_operand, operand, len_of_operand);
        (instruction->operands).type = TYPE_STRING;
        //printf("(instruction->label).label_name = %s\n", (instruction->label).label_name);
        *pc = find_label_and_change_it(commands, (instruction->label).label_name, len_of_operand, SECTION_DATA, LABEL_WITH_DATA, len_of_operand);
        //*pc += len_of_operand;
        offset = len_of_operand;
    }
    instruction->pc = *pc;
    *pc += offset;
    return;
}

static void get_all_functions(struct Binary_file *binary_struct, FILE *file_pointer_with_commands)
{
    if (binary_struct == NULL || file_pointer_with_commands == NULL)
    {
        fprintf(stderr, "ERROR IN \x1b[33mget_all_functions!\x1b[0m\n");
        abort();
    }
    char all_functions[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    fgets(all_functions, sizeof(all_functions), file_pointer_with_commands);
    if (all_functions[0] != ';')
    {
        fprintf(stderr, "Error! There is no string with all functions\n");
        abort();
    }
    size_t count_of_functions = 0;
    sscanf(all_functions + 1, "%lu", &count_of_functions);
    binary_struct->all_functions = (struct Function_type *) calloc (count_of_functions, sizeof(struct Function_type));
    if (binary_struct->all_functions == NULL)
    {
        fprintf(stderr, "Error! This is impossible to create binary_struct->all_functions\n");
        abort();
    }
    binary_struct->count_of_functions = count_of_functions;
    size_t position = 2;
    char function_name[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    for (size_t index = 0; index < count_of_functions; index++)
    {
        sscanf(all_functions + position, "%s", ((binary_struct->all_functions)[index]).name);
        position += strlen(((binary_struct->all_functions)[index]).name) + 1;
    }

    return;
}

static bool check_if_function(struct Binary_file *binary_struct, const char *label_name)
{
    if (binary_struct == NULL || label_name == NULL)
    {
        fprintf(stderr, "ERROR IN \x1b[33mcheck_if_function \x1b[0m \n");
        abort();
    }
    for (size_t index = 0; index < (binary_struct->count_of_functions); index++)
    {
        if (strcasecmp((binary_struct->all_functions)[index].name, label_name) == 0)
        {
            return true;
        }
    }
    return false;
}

static bool is_str_digit(const char *str)
{
    while (*str)
    {
        if (!isdigit(*str))
        {
            return false;
        }
        str++;
    }
    return true;
}

static void add_relocation(struct Binary_file *binary_struct, size_t offset_in_text, const char *symbol_name, uint32_t type, int64_t additional_offset)
{
    binary_struct->relocation_table = (struct Relocation*) realloc(binary_struct->relocation_table, (binary_struct->size_of_relocation_table + 1) * sizeof(struct Relocation));
    if (binary_struct->relocation_table == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m Error of null relocation table after realloc\n");
        abort();
    }
    Relocation *rel = &((binary_struct->relocation_table)[binary_struct->size_of_relocation_table]);
    memset(rel, 0, sizeof(*rel));
    rel->offset_in_text = offset_in_text;
    rel->type = type;
    rel->additional_offset = additional_offset;
    strncpy(rel->symbol_name, symbol_name, sizeof(rel->symbol_name) - 1);
    rel->symbol_name[sizeof(rel->symbol_name) - 1] = '\0';
    (binary_struct->size_of_relocation_table)++;
    return;
}

static void encode_text_instructions(struct Data_CMDS *commands, struct Binary_file *binary_struct)
{
    if (commands == NULL || binary_struct == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m Error in \x1b[33mencode_text_instructions\x1b[0m\n");
        abort();
    }
    binary_struct->size_of_text_labels = commands->size_of_labels_text;
    binary_struct->text_labels = (struct Label *) calloc (binary_struct->size_of_text_labels, sizeof(struct Label));
    memcpy(binary_struct->text_labels, commands->labels_text, binary_struct->size_of_text_labels);
    size_t buffer_of_commands_size = 0;
    for (size_t index = 0; index < (commands->size_of_instructions); index++)
    {
        Instruction *instruction = &((commands->instructions)[index]);
        //printf("index = %lu\nopcode = %d\n",index, instruction->opcode);
        if (instruction->section != SECTION_TEXT)
        {
            continue;
        }
        uint8_t *pointer_in_buffer_cmds = (binary_struct->buffer_of_text_commands) + buffer_of_commands_size;
        if (instruction->opcode == OP_MOV_REG_IMM)
        {
            //printf("imm = %lu\n", instruction->imm);
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;

            //for ||mov reg, imm|| opcode = 0xB8
            *pointer_in_buffer_cmds = 0xB8 + (instruction->register_dest & 0x7);
            pointer_in_buffer_cmds++;
            //imm little_endian
            uint64_t imm = (uint64_t) instruction->imm;
            memcpy(pointer_in_buffer_cmds, &imm, 8);
            pointer_in_buffer_cmds += 8;
            buffer_of_commands_size += 1/*opcode*/ + 8/*imm64*/ + 1/*REX*/;
        }
        else if (instruction->opcode == OP_MOV_REG_REG)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            //printf("register_dest = %d\nregister_source = %d\n", instruction->register_dest, instruction->register_source);
            *pointer_in_buffer_cmds = 0x89;//for ||mov reg, reg|| opcode = 0x89
            pointer_in_buffer_cmds++;
            uint8_t modrm = 0xC0;//mod = 11b - register mode
            modrm |= ((instruction->register_source & 0x7) << 3);
            modrm |= (instruction->register_dest & 0x7);
            *pointer_in_buffer_cmds = modrm;
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 1 + 1 + 1;
        }
        else if (instruction->opcode == OP_MOV_REG_LABEL)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;

            //for ||mov reg, imm|| opcode = 0xB8
            *pointer_in_buffer_cmds = 0xB8 + (instruction->register_dest & 0x7);
            pointer_in_buffer_cmds++;
            int64_t zero = 0;
            memcpy(pointer_in_buffer_cmds, &zero, 8);
            pointer_in_buffer_cmds += 8;
            buffer_of_commands_size += 1 + 1 + 8;
            add_relocation(binary_struct, buffer_of_commands_size - 8, (instruction->label).label_name, R_X86_64_64, 0);
        }
        else if (instruction->opcode == OP_MOV_REG_ADDRESS_REG)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            *pointer_in_buffer_cmds = 0x8B;
            pointer_in_buffer_cmds++;
            uint8_t modrm = 0x00;//mod = 00b - mode r/m
            modrm |= ((instruction->register_dest & 0x7) << 3);
            modrm |= (instruction->register_source & 0x7);
            *pointer_in_buffer_cmds = modrm;
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 1 + 1 + 1;
        }
        else if (instruction->opcode == OP_MOV_REG_ADDRESS_LABEL)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            *pointer_in_buffer_cmds = 0x8B;
            pointer_in_buffer_cmds++;
            uint8_t modrm = 0x00;//mod = 00b - mode r/m
            modrm |= ((instruction->register_dest & 0x7) << 3);
            modrm |= 0x5;
            *pointer_in_buffer_cmds = modrm;
            pointer_in_buffer_cmds++;
            int64_t zero = 0;
            memcpy(pointer_in_buffer_cmds, &zero, 4);
            pointer_in_buffer_cmds += 4;
            buffer_of_commands_size += 1 + 1 + 1 + 4;
            add_relocation(binary_struct, buffer_of_commands_size - 4, (instruction->label).label_name, R_X86_64_PC32, -4);
        }
        else if (instruction->opcode == OP_LEA_REG_ABS_ADDRESS_REG)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            *pointer_in_buffer_cmds = 0x8D;//Opcode of ||lea||
            pointer_in_buffer_cmds++;
            uint8_t modrm = 0x00;//mod = 00b - mode r/m
            modrm |= ((instruction->register_dest & 0x7) << 3);
            modrm |= (instruction->register_source & 0x7);
            *pointer_in_buffer_cmds = modrm;
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 1 + 1 + 1;
        }
        else if (instruction->opcode == OP_LEA_REG_ABS_ADDRESS_LABEL)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            *pointer_in_buffer_cmds = 0x8D;//Opcode of ||lea||
            pointer_in_buffer_cmds++;
            uint8_t modrm = 0x00;//mod = 00b - mode r/m
            modrm |= ((instruction->register_dest & 0x7) << 3);
            modrm |= 0x5;
            *pointer_in_buffer_cmds = modrm;
            pointer_in_buffer_cmds++;
            int64_t zero = 0;
            memcpy(pointer_in_buffer_cmds, &zero, 4);
            pointer_in_buffer_cmds += 4;
            buffer_of_commands_size += 1 + 1 + 1 + 4;
            add_relocation(binary_struct, buffer_of_commands_size - 4, (instruction->label).label_name, R_X86_64_PC32, -4);
        }
        else if (instruction->opcode == OP_XOR_REG_REG)
        {
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            *pointer_in_buffer_cmds = 0x31;
            pointer_in_buffer_cmds++;
            uint8_t modrm = 0xC0;//mod = 11b - register mode
            modrm |= ((instruction->register_source & 0x7) << 3);
            modrm |= (instruction->register_dest & 0x7);
            *pointer_in_buffer_cmds = modrm;
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 1 + 1 + 1;
        }
        else if (instruction->opcode == OP_PUSH_REG)
        {
            //printf("register = %d\n", instruction->register_source);
            *pointer_in_buffer_cmds = 0x50 + (instruction->register_source & 0x7);
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 1;
        }
        else if (instruction->opcode == OP_POP_TO_REG)
        {
            //printf("index = %lu\n", index);
            //printf("register = %d\n", instruction->register_dest);
            *pointer_in_buffer_cmds = 0x58 + (instruction->register_dest & 0x7);
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 1;
        }
        else if (instruction->opcode == OP_SYSCALL)
        {
            //printf("Here\n");
            *pointer_in_buffer_cmds = 0x0F;
            pointer_in_buffer_cmds++;
            *pointer_in_buffer_cmds = 0x05;
            pointer_in_buffer_cmds++;
            buffer_of_commands_size += 2;
        }
    }
    binary_struct->len_buffer_of_text_commands = buffer_of_commands_size;
    //printf("len_of_buffer_text_commands = %lu\n", binary_struct->len_buffer_of_text_commands);
    return;
}

static void encode_data_instructions(struct Data_CMDS *commands, struct Binary_file *binary_struct)
{
    if (commands == NULL || binary_struct == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m Error in \x1b[33mencode_data_instructions\x1b[0m\n");
        abort();
    }
    
    binary_struct->size_of_data_labels = commands->size_of_labels_data;
    binary_struct->data_labels = (struct Label *) calloc (binary_struct->size_of_data_labels, sizeof(struct Label));
    //FIX IT LATER
    for (size_t index = 0; index < commands->size_of_labels_data; index++)
    {
        //printf("label_name = %s\n", (commands->labels_data)[index].label_name);
        strcpy((binary_struct->data_labels)[index].label_name, (commands->labels_data)[index].label_name);
        (binary_struct->data_labels)[index].imm_data = (commands->labels_data)[index].imm_data;
        (binary_struct->data_labels)[index].pc = (commands->labels_data)[index].pc;
        (binary_struct->data_labels)[index].size_of_data = (commands->labels_data)[index].size_of_data;
        (binary_struct->data_labels)[index].type = (commands->labels_data)[index].type;
    }
    //
    size_t buffer_of_commands_size = 0;
    for (size_t index = 0; index < (commands->size_of_instructions); index++)
    {
        Instruction *instruction = &((commands->instructions)[index]);
        if (instruction->section != SECTION_DATA)
        {
            continue;
        }
        uint8_t *pointer_in_buffer_cmds = (binary_struct->buffer_of_data_commands) + buffer_of_commands_size;
        size_t size_of_operand = (size_t)(instruction->initial_instruction);
        if ((instruction->operands).type == TYPE_NUMBER)
        {
            //printf("number\n");
            for (size_t id = 0; id < size_of_operand; id++)
            {
                *pointer_in_buffer_cmds = (uint8_t) (((instruction->operands).numeric_operand >> (8 * id)) & 0xFF);
                pointer_in_buffer_cmds++;
            }
            buffer_of_commands_size += size_of_operand;
        }
        else
        {
            //printf("operand = %s\n", (instruction->operands).string_operand);
            size_t len_of_string = (instruction->operands).len_of_string_operand;
            //printf("len_of_string = %lu\n", len_of_string);
            for (size_t id = 0; id < len_of_string; id++)
            {
                *pointer_in_buffer_cmds = (uint8_t) ((instruction->operands).string_operand[id]);
                pointer_in_buffer_cmds++;
                buffer_of_commands_size += size_of_operand;
            }
        }
    }
    binary_struct->len_buffer_of_data_commands = buffer_of_commands_size;
    return;
}