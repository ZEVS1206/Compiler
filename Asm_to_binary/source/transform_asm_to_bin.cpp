#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <elf.h>

#include "function_and_structures.h"

const uint8_t buffer_hex[] = {{0x00},
                              {0x01},
                              {0x02},
                              {0x03},
                              {0x04},
                              {0x05},
                              {0x06},
                              {0x07},
                              {0x08},
                              {0x09},
                              {0x0A},
                              {0x0B},
                              {0x0C},
                              {0x0D},
                              {0x0E},
                              {0x0F}
                             };
const size_t size_of_buffer_hex_to_bin = sizeof(buffer_hex) / sizeof(uint8_t);

const struct CMD cmd_to_byte[] = {{"mov", {0xB0}},
                                  {"rax", {0x08}},
                                  {"ret", {0x3C}}
                                 };
const size_t size_of_cmd_to_byte = sizeof(cmd_to_byte) / sizeof(struct CMD);

static void dec_to_bytes(const char *dec_number, uint8_t **result);
static size_t align_up(size_t offset, size_t base);//Need for align_up offset in memory
static unsigned add_string_to_buffer(char *buffer, size_t *len_of_buffer, const char *string);//Function for adding string to buffer
Errors_of_binary create_header(struct Binary_file *binary_struct);

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


static void dec_to_bytes(const char *dec_number, uint8_t **result)
{
    if (dec_number == NULL)
    {
        fprintf(stderr, "Error! Null dec_number was transfered to function dec_to_bytes\n");
        abort();
    }
    int number = atoi(dec_number);
    int position = 0;
    uint8_t converted_number[50] = {};
    int ost = 0;
    while (number > 0)
    {
        ost = number % 16;
        converted_number[position++] = buffer_hex[ost];
        number /= 16;
    }
    position--;
    while (position >= 0)
    {
        (*result)[0] = converted_number[position--];
        (*result)++;
    }
    return;
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
    struct Binary_file binary_struct = {0};
    binary_struct.buffer_of_commands = (uint8_t *) calloc (6, sizeof(uint8_t));
    binary_struct.len_buffer_of_commands = 6;
    if (binary_struct.buffer_of_commands == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    binary_struct.buffer_of_commands[0] = 0xB8;
    binary_struct.buffer_of_commands[1] = 0x2A;
    binary_struct.buffer_of_commands[2] = 0x00;
    binary_struct.buffer_of_commands[3] = 0x00;
    binary_struct.buffer_of_commands[4] = 0x00;
    binary_struct.buffer_of_commands[5] = 0xC3;
    binary_struct.file_pointer = file_out_pointer;
    Errors_of_binary error = create_header(&binary_struct);
    if (error != NO_BINARY_ERRORS)
    {
        return error;
    }
    free(binary_struct.buffer_of_commands);
    fclose(file_out_pointer);
    return NO_BINARY_ERRORS;
}

Errors_of_binary create_header(struct Binary_file *binary_struct)
{
    if (binary_struct == NULL)
    {
        return ERROR_OF_CREATING_HEADER;
    }
    binary_struct->len_section_header_str_table = 1;
    (binary_struct->section_header_str_table)[0] = '\0';
    unsigned section_header_name_text     = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".text");
    unsigned section_header_name_shstrtab = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".shstrtab");
    unsigned section_header_name_symtab   = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".symtab");
    unsigned section_header_name_strtab   = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".strtab");
    unsigned section_header_name_gnustack = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".note.GNU-stack");
    binary_struct->len_symbol_string_table = 1;
    (binary_struct->symbol_string_table)[0] = '\0';
    unsigned string_name_file  = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), "transform_asm_to_bin.cpp");
    unsigned string_name_func  = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), "func");
    const int SYM_COUNT = 4;
    Elf64_Sym syms[SYM_COUNT] = {0};
    // [1] file
    syms[1].st_name = string_name_file;
    syms[1].st_info = ELF64_ST_INFO(STB_LOCAL, STT_FILE);
    syms[1].st_shndx = SHN_ABS;
    // [2] section .text
    syms[2].st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION);
    // [3] myfunc
    syms[3].st_name = string_name_func;
    syms[3].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
    syms[3].st_shndx = 1;    // will be .text index (we place .text as sec idx 1)
    syms[3].st_value = 0;    // offset in .text
    syms[3].st_size  = (Elf64_Xword)(binary_struct->len_buffer_of_commands);
    const int LOCAL_COUNT = 3; // symbols 0..2 are local -> first non-local has index 3
    // 4) layout sections in file (indices: 0=NULL,1=.text,2=.shstrtab,3=.symtab,4=.strtab,5=.note.GNU-stack)
    const int SHNUM = 6;
    Elf64_Shdr sh[SHNUM] = {0};
    size_t offset = sizeof(Elf64_Ehdr);
    // .text
    offset = align_up(offset, 16); 
    size_t text_offset = offset; 
    offset += binary_struct->len_buffer_of_commands;
    // .shstrtab
    offset = align_up(offset, 1); 
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

    //.text
    sh[1].sh_name = section_header_name_text;
    sh[1].sh_type = SHT_PROGBITS;
    sh[1].sh_flags = SHF_ALLOC | SHF_EXECINSTR;
    sh[1].sh_offset = text_offset;
    sh[1].sh_size = binary_struct->len_buffer_of_commands;
    sh[1].sh_addralign = 16;

    //.shstrtab
    sh[2].sh_name = section_header_name_shstrtab;
    sh[2].sh_type = SHT_STRTAB;
    sh[2].sh_offset = shstr_offset;
    sh[2].sh_size = binary_struct->len_section_header_str_table;
    sh[2].sh_addralign = 1;

    //.symtab
    sh[3].sh_name = section_header_name_symtab;
    sh[3].sh_type = SHT_SYMTAB;
    sh[3].sh_offset = symtab_offset;
    sh[3].sh_size = symtab_size;
    sh[3].sh_link = 4; // .strtab index
    sh[3].sh_info = LOCAL_COUNT;
    sh[3].sh_addralign = 8;
    sh[3].sh_entsize = sizeof(Elf64_Sym);

    //.strtab
    sh[4].sh_name = section_header_name_strtab;
    sh[4].sh_type = SHT_STRTAB;
    sh[4].sh_offset = strtab_offset;
    sh[4].sh_size = binary_struct->len_symbol_string_table;
    sh[4].sh_addralign = 1;

    //.note.GNU-stack(empty)
    sh[5].sh_name = section_header_name_gnustack;
    sh[5].sh_type = SHT_PROGBITS;
    sh[5].sh_offset = gnustack_offset;
    sh[5].sh_size = gnustack_size;
    sh[5].sh_addralign = 1;

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
    (binary_struct->ELF_Header).e_shstrndx = 2; // index .shstrtab

    fwrite(&(binary_struct->ELF_Header), 1, sizeof(binary_struct->ELF_Header), binary_struct->file_pointer);
    fseek(binary_struct->file_pointer, (long)text_offset, SEEK_SET); 
    fwrite(binary_struct->buffer_of_commands, 1, binary_struct->len_buffer_of_commands, binary_struct->file_pointer);
    fseek(binary_struct->file_pointer, (long)shstr_offset, SEEK_SET); 
    fwrite(binary_struct->section_header_str_table, 1, binary_struct->len_section_header_str_table, binary_struct->file_pointer);
    fseek(binary_struct->file_pointer, (long)strtab_offset, SEEK_SET); 
    fwrite(binary_struct->symbol_string_table, 1, binary_struct->len_symbol_string_table, binary_struct->file_pointer);
    fseek(binary_struct->file_pointer, (long)symtab_offset, SEEK_SET); 
    fwrite(syms, 1, symtab_size, binary_struct->file_pointer);
    // .note.GNU-stack empty
    fseek(binary_struct->file_pointer, (long)section_header_offset, SEEK_SET); 
    fwrite(sh, 1, section_header_table_size, binary_struct->file_pointer);
    return NO_BINARY_ERRORS;
}