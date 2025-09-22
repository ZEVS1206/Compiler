#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <elf.h>

#include "function_and_structures.h"

#define MAX_LEN_FOR_STATIC_ARRAYS 100

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
static Errors_of_binary create_binary_file(struct Binary_file *binary_struct);
static Errors_of_binary create_bin_data_for_binary_file(struct Binary_file *binary_struct, FILE *file_pointer_with_commands);
static void get_all_functions(struct Binary_file *binary_struct, FILE *file_pointer_with_commands);
static size_t get_size_of_file(FILE *file_pointer);
static Register get_register(const char *register_name);
static void add_label(struct Data_CMDS *commands, const char *label_name, int32_t pc);
static int find_label(struct Data_CMDS *commands, const char *label_name);
static void encode_instructions(struct Data_CMDS *commands, struct Binary_file *binary_struct);
static bool check_if_function(struct Binary_file *binary_struct, const char *label_name);
static bool is_str_digit(const char *str);
static char * skip_spaces(char *buffer, char *end_pointer);


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
    uint8_t converted_number[MAX_LEN_FOR_STATIC_ARRAYS] = {};
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

    // uint8_t *result = (uint8_t *) calloc (50, sizeof(uint8_t));
    // if (result == NULL)
    // {
    //     return ERROR_OF_OPERATING_CMD;
    // }
    // uint8_t *old_pointer = &(result[0]);
    // dec_to_bytes("255", &result);
    // result = old_pointer;
    // for (size_t i = 0; i < 3; i++)
    // {
    //     printf("%x", result[i]);
    // }
    // printf("\n");
    struct Binary_file binary_struct = {0};
    Errors_of_binary error = create_bin_data_for_binary_file(&binary_struct, file_pointer);
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
    free(binary_struct.buffer_of_commands);
    free(binary_struct.all_functions);
    fclose(file_out_pointer);
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
    unsigned section_header_name_text     = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".text");
    unsigned section_header_name_shstrtab = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".shstrtab");
    unsigned section_header_name_symtab   = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".symtab");
    unsigned section_header_name_strtab   = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".strtab");
    unsigned section_header_name_gnustack = add_string_to_buffer(binary_struct->section_header_str_table, &(binary_struct->len_section_header_str_table), ".note.GNU-stack");
    binary_struct->len_symbol_string_table = 1;
    (binary_struct->symbol_string_table)[0] = '\0';
    unsigned string_name_file  = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), "transform_asm_to_bin.cpp");
    for (size_t index = 0; index < binary_struct->count_of_functions; index++)
    {
        (binary_struct->all_functions)[index].position_in_strtab = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), (binary_struct->all_functions)[index].name);
    }
    //unsigned string_name_func  = add_string_to_buffer(binary_struct->symbol_string_table, &(binary_struct->len_symbol_string_table), "func");
    const int SYM_COUNT = 3 + binary_struct->count_of_functions;
    Elf64_Sym *syms = (Elf64_Sym *) calloc (SYM_COUNT, sizeof(Elf64_Sym));
    if (syms == NULL)
    {
        return ERROR_OF_CREATING_HEADER;
    }
    // [1] file
    syms[1].st_name = string_name_file;
    syms[1].st_info = ELF64_ST_INFO(STB_LOCAL, STT_FILE);
    syms[1].st_shndx = SHN_ABS;
    // [2] section .text
    syms[2].st_info = ELF64_ST_INFO(STB_LOCAL, STT_SECTION);
    // [3-...] myfunctions
    size_t offset_in_text = 0;
    for (size_t index = 3; index < SYM_COUNT; index++)
    {
        if (index - 3 > binary_struct->count_of_functions)
        {
            break;
        }
        syms[index].st_name = (binary_struct->all_functions)[index - 3].position_in_strtab;
        syms[index].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
        syms[index].st_shndx = 1; // will be .text index (we place .text as sec idx 1)
        syms[index].st_value = offset_in_text;
        syms[index].st_size = (Elf64_Xword)(binary_struct->all_functions)[index - 3].offset_in_text;
        offset_in_text += (binary_struct->all_functions)[index - 3].offset_in_text;
    }
    // syms[3].st_name = string_name_func;
    // syms[3].st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
    // syms[3].st_shndx = 1;    // will be .text index (we place .text as sec idx 1)
    // syms[3].st_value = 0;    // offset in .text
    // syms[3].st_size  = (Elf64_Xword)(binary_struct->len_buffer_of_commands);
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
    if (strcasecmp(register_name, "eax") == 0)
    {
        return EAX;
    }
    if (strcasecmp(register_name, "ebx") == 0)
    {
        return EBX;
    }
    if (strcasecmp(register_name, "ecx") == 0)
    {
        return ECX;
    }
    if (strcasecmp(register_name, "edx") == 0)
    {
        return EDX;
    }
    return UNKNOWN_REGISTER;
}

static void add_label(struct Data_CMDS *commands, const char *label_name, int32_t pc)
{
    if (commands == NULL)
    {
        fprintf(stderr, "Error! There is no data commands\n");
        abort();
    }
    for (size_t index = 0; index < (commands->size_of_labels); index++)
    {
        if (strcasecmp(((commands->labels)[index]).label_name, label_name) == 0)
        {
            fprintf(stderr, "Dublicate label: %s\n", label_name);
            abort();
        }
    }
    strncpy(((commands->labels)[commands->size_of_labels]).label_name, label_name, strlen(label_name));
    ((commands->labels)[commands->size_of_labels]).pc = pc;
    (commands->size_of_labels)++;
    return;
}

static int find_label(struct Data_CMDS *commands, const char *label_name)
{
    if (commands == NULL)
    {
        fprintf(stderr, "Error! There is no data commands\n");
        abort();
    }
    for (size_t index = 0; index < (commands->size_of_labels); index++)
    {
        if (strcasecmp((commands->labels)[index].label_name, label_name) == 0)
        {
            return ((commands->labels)[index].pc);
        }
    }
    return -1;
}



static Errors_of_binary create_bin_data_for_binary_file(struct Binary_file *binary_struct, FILE *file_pointer_with_commands)
{
    if (binary_struct == NULL || file_pointer_with_commands == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    size_t size_of_file = get_size_of_file(file_pointer_with_commands);
    char str_for_input[MAX_LEN_FOR_STATIC_ARRAYS] = "";
    char *end_pointer = (str_for_input + MAX_LEN_FOR_STATIC_ARRAYS);
    int32_t pc = 0;//programm counter
    struct Data_CMDS commands = {0};
    commands.instructions = (struct Instruction *) calloc (size_of_file, sizeof(struct Instruction));
    if (commands.instructions == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    commands.labels = (struct Label *) calloc (size_of_file, sizeof(struct Label));
    if (commands.labels == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    commands.size_of_instructions = 0;
    commands.size_of_labels = 0;
    get_all_functions(binary_struct, file_pointer_with_commands);
    int index_of_last_function = -1;
    while(fgets(str_for_input, sizeof(str_for_input), file_pointer_with_commands))
    {
        //printf("str for input = %s\n", str_for_input);
        char *start = str_for_input;
        start = skip_spaces(start, end_pointer);
        if (*start == '\0' || *start == ';')
        {
            continue;
        }
        char label_name[MAX_LEN_FOR_STATIC_ARRAYS] = "";
        if (sscanf(start, "%s", label_name) == 1)
        {
            size_t len = strlen(label_name);
            bool is_label = false;
            for (size_t id = 0; id < len; id++)
            {
                if (label_name[id] == ':')
                {
                    is_label = true;
                    break;
                }
            }
            char new_label_name[MAX_LEN_FOR_STATIC_ARRAYS] = "";
            snprintf(new_label_name, len, "%s", label_name);
            if (is_label)
            {
                bool result = check_if_function(binary_struct, new_label_name);
                if (!result)
                {
                    add_label(&commands, new_label_name, pc);
                    continue;
                }
                if (index_of_last_function == -1)
                {
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
                    (binary_struct->all_functions)[index_of_last_function].offset_in_text = pc;
                    for (size_t index = 0; index < (binary_struct->count_of_functions); index++)
                    {
                        if (strcasecmp((binary_struct->all_functions)[index].name, new_label_name) == 0)
                        {
                            index_of_last_function = index;
                            break;
                        }
                    }
                }
            }
        }
        struct Instruction instruction = {};
        instruction.pc = pc;
        start = skip_spaces(start, end_pointer);
        char operation[MAX_LEN_FOR_STATIC_ARRAYS] = "";
        if (sscanf(start, "%s", operation) != 1)
        {
            continue;
        }
        start += strlen(operation);
        if (strcasecmp(operation, "mov") == 0)
        {
            
            char param_1[8] = "";
            char param_2[8] = "";
            int res = sscanf(start, "%7[^,]", param_1);
            start += strlen(param_1);
            start += 2;
            res += sscanf(start, "%s", param_2);
            if (res != 2)
            {
                fprintf(stderr, "\x1b[31mError!\x1b[0m Error in parsing mov\n");
                abort();
            }
            bool is_digit = is_str_digit(param_2);
            int64_t imm = 0;
            if (is_digit)
            {
                char *end = NULL;
                imm = strtol(param_2, &end, 10);
            }
            instruction.opcode = OP_MOV_REG_IMM;
            instruction.register_dest = get_register(param_1);
            if (is_digit)
            {
                instruction.register_source = UNKNOWN_REGISTER;
                instruction.imm = imm;
                pc += 1/*opcode*/ + 8/*imm64*/ + 1/*REX*/;
            }
            else
            {
                instruction.register_source = get_register(param_2);
                pc += 1/*opcode*/ + 1/*ModR/M*/ + 1/*REX*/;
            }
        }
        commands.instructions[commands.size_of_instructions++] = instruction;
        if (commands.size_of_instructions >= size_of_file)
        {
            fprintf(stderr, "\x1b[31mError!\x1b[0m Not enough memory in \x1b[33mcommands.instructions\x1b[0m\n");
            abort();
        }
    }
    (binary_struct->all_functions)[index_of_last_function].offset_in_text = pc;
    binary_struct->buffer_of_commands = (uint8_t *) calloc (size_of_file, sizeof(uint8_t));
    if (binary_struct->buffer_of_commands == NULL)
    {
        return ERROR_OF_CREATING_BUFFER_OF_CMDS;
    }
    binary_struct->len_buffer_of_commands = 0;
    encode_instructions(&commands, binary_struct);

    free(commands.instructions);
    free(commands.labels);
    return NO_BINARY_ERRORS;
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

static void encode_instructions(struct Data_CMDS *commands, struct Binary_file *binary_struct)
{
    if (commands == NULL || binary_struct == NULL)
    {
        fprintf(stderr, "\x1b[31mError!\x1b[0m Error in \x1b[33mencode_instructions\x1b[0m\n");
        abort();
    }
    size_t buffer_of_commands_size = 0;
    for (size_t index = 0; index < (commands->size_of_instructions); index++)
    {
        Instruction *instruction = &((commands->instructions)[index]);
        uint8_t *pointer_in_buffer_cmds = (binary_struct->buffer_of_commands) + buffer_of_commands_size;
        if (instruction->opcode == OP_MOV_REG_IMM)
        {
            //Prefix REX
            *pointer_in_buffer_cmds = 0x48;
            pointer_in_buffer_cmds++;
            //opcode b8 + code of register
            
            if (instruction->register_source == UNKNOWN_REGISTER)
            {
                *pointer_in_buffer_cmds = 0xB8 + (instruction->register_dest & 0x7);
                pointer_in_buffer_cmds++;
                uint64_t imm = (uint64_t) instruction->imm;
                memcpy(pointer_in_buffer_cmds, &imm, 8);
                pointer_in_buffer_cmds += 8;
                buffer_of_commands_size += 1 + 8 + 1;
            } 
            // else
            // {
            //     *pointer_in_buffer_cmds = 0xB8;

            //     uint8_t modrm = 0xC0;
            //     modrm |= ((instruction->register_dest & 0x7) << 3);
            //     modrm |= ((instruction->register_source) & 0x7);
            //     *pointer_in_buffer_cmds = modrm;
            //     pointer_in_buffer_cmds++;
            //     buffer_of_commands_size += 1 + 1 + 1;
            // }
        }
    }
    binary_struct->len_buffer_of_commands = buffer_of_commands_size;
    return;
}