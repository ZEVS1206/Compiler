#ifndef FUNCTION_AND_STRUCTURES_H
#define FUNCTION_AND_STRUCTURES_H

#pragma once

#include <stdio.h>
#include <stdint.h>
#include <elf.h>

enum Errors_of_binary
{
    NO_BINARY_ERRORS                 = 0,
    ERROR_OF_CREATING_OUT_FILE       = 1,
    ERROR_OF_OPERATING_CMD           = 2,
    ERROR_OF_CREATING_HEADER         = 3,
    ERROR_OF_CREATING_BUFFER_OF_CMDS = 4,
    ERROR_OF_CONSTRUCTOR             = 5,
    ERROR_OF_DESTRUCTOR              = 6,
    ERROR_OF_READING_CMDS            = 7,
    ERROR_OF_PREPROCESS_SOURCE_FILE  = 8
};

struct CMD
{
    char original_name[50];
    uint8_t byte_code[16];
};

struct Function_type
{
    char name[50];
    unsigned position_in_strtab;
    size_t offset_in_text;
};

struct Relocation
{
    size_t offset_in_text;
    char symbol_name[100];
    uint32_t type;
    int64_t additional_offset;
    int symbol_index;
};

struct Binary_file
{
    FILE *file_pointer;
    size_t count_of_functions;
    struct Function_type *all_functions;
    char section_header_str_table[256];//Table of names of sections
    size_t len_section_header_str_table;
    char symbol_string_table[256];//Table of names of symbols
    size_t len_symbol_string_table;
    Elf64_Ehdr ELF_Header;
    uint8_t *buffer_of_text_commands;
    size_t len_buffer_of_text_commands;
    uint8_t *buffer_of_data_commands;
    size_t len_buffer_of_data_commands;
    struct Relocation *relocation_table;
    size_t size_of_relocation_table;
    struct Label *data_labels;
    struct Label *text_labels;
    size_t size_of_data_labels;
    size_t size_of_text_labels;
    size_t count_of_bytes_before_start;
};

enum Opcode
{
    UNKNOWN_OPCODE               = 0,
    OP_MOV_REG_IMM               = 1,
    OP_JMP_LABEL                 = 2,
    OP_MOV_REG_REG               = 3,
    OP_XOR_REG_REG               = 4,
    OP_XOR_REG_IMM               = 5,
    OP_SYSCALL                   = 6,
    OP_MOV_REG_LABEL             = 7,
    OP_PUSH_REG                  = 8,
    OP_POP_TO_REG                = 9,
    OP_LEA_REG_ABS_ADDRESS_REG   = 10,
    OP_LEA_REG_ABS_ADDRESS_LABEL = 11,
    OP_MOV_REG_ADDRESS_REG       = 12,
    OP_MOV_REG_ADDRESS_LABEL     = 13,
    OP_ADD_REG_IMM               = 14,
    OP_ADD_REG_REG               = 15,
    OP_SUB_REG_IMM               = 16,
    OP_SUB_REG_REG               = 17,
    OP_MUL_REG                   = 18,
    OP_IMUL_REG_REG              = 19,
    OP_DIV_REG                   = 20,
    OP_IDIV_REG                  = 21,
    OP_MOV_MEMORY_REG            = 22,
    OP_MOV_MEMORY_IMM            = 23,
    OP_INC_REG                   = 24,
    OP_DEC_REG                   = 25,
    OP_CMP_REG_IMM               = 26,
    OP_CMP_REG_REG               = 27,
};

enum Jmp_cmds
{
    UNKNOWN_JMP_CMD = 0,
    JMP             = 1,
    JG              = 2,//>
    JGE             = 3,//>=
    JL              = 4,//<
    JLE             = 5,//<=
    JE              = 6,//==
    JNE             = 7 //!=
};
enum Register_x8
{
    AL                  = 0,
    CL                  = 1,
    DL                  = 2,
    BL                  = 3,
    AH                  = 4,
    CH                  = 5,
    DH                  = 6,
    BH                  = 7,
    UNKNOWN_REGISTER_X8 = 16
};

enum Register_x16
{
    AX                   = 0,
    CX                   = 1,
    DX                   = 2,
    BX                   = 3,
    SP                   = 4,
    BP                   = 5,
    SI                   = 6,
    DI                   = 7,
    UNKNOWN_REGISTER_X16 = 16
};

enum Register_x32
{
    EAX                  = 0,
    ECX                  = 1,
    EDX                  = 2,
    EBX                  = 3,
    ESP                  = 4,
    EBP                  = 5,
    ESI                  = 6,
    EDI                  = 7,
    E8                   = 8,
    E9                   = 9,
    E10                  = 10,
    E11                  = 11,
    E12                  = 12,
    E13                  = 13,
    E14                  = 14,
    E15                  = 15,
    UNKNOWN_REGISTER_X32 = 16
};

enum Register_x64
{
    RAX                  = 0,
    RCX                  = 1,
    RDX                  = 2,
    RBX                  = 3,
    RSP                  = 4,
    RBP                  = 5,
    RSI                  = 6,
    RDI                  = 7,
    R8                   = 8,
    R9                   = 9,
    R10                  = 10,
    R11                  = 11,
    R12                  = 12,
    R13                  = 13,
    R14                  = 14,
    R15                  = 15,
    UNKNOWN_REGISTER_X64 = 16
};

enum Register_type
{
    UNKNOWN_REGISTER_TYPE = 0,
    TYPE_X8               = 1,
    TYPE_X16              = 2,
    TYPE_X32              = 3,
    TYPE_X64              = 4
};

struct Register
{
    Register_type type;
    Register_x8  register_x8;
    Register_x16 register_x16;
    Register_x32 register_x32;
    Register_x64 register_x64;
};

enum Sections
{
    UNKNOWN_SECTION = 0,
    SECTION_DATA    = 1,
    SECTION_TEXT    = 2
};
enum Initial_instructions
{
    UNKNOWN_INSTRUCTION = 0,
    INSTRUCTION_DB      = 1,
    INSTRUCTION_DW      = 2,
    INSTRUCTION_DD      = 4,
    INSTRUCTION_DQ      = 8,
    INSTRUCTION_EQU     = 5
};

enum Type_of_operand
{
    UNKNOWN_TYPE = 0,
    TYPE_NUMBER  = 1,
    TYPE_STRING  = 2
};

struct Instruction_operand
{
    Type_of_operand type;
    size_t len_of_string_operand;
    union
    {
        char symbol_operand;
        uint8_t string_operand[100];
        int64_t numeric_operand;
    };  
};

enum Type_of_label
{
    NOT_A_LABEL     = 0,
    CONSTANT_LABEL  = 1,
    LABEL_WITH_DATA = 2,
    LABEL_FOR_JMP   = 3
};

enum Type_of_data
{
    UNKNOWN_DATA_TYPE = 0,
    NUMBER            = 1,
    BUFFER_STR        = 2
};

enum Size_of_imm
{
    UNKNOWN_IMM  = 0,
    SIZE_IMM_8   = 1,
    SIZE_IMM_16  = 2,
    SIZE_IMM_32  = 4,
    SIZE_IMM_64  = 8
};

struct Label
{
    char label_name[100];
    int32_t pc;
    size_t size_of_data;
    Type_of_label type;
    int64_t imm_data;
    unsigned position_in_strtab;
    Type_of_data type_of_data;
    bool label_before_use;
};

struct Instruction
{
    Initial_instructions initial_instruction;
    struct Instruction_operand operands;
    Sections section;
    Opcode opcode;
    Register register_dest;//destination / first parametre
    Register register_source;// source / second parametre
    int64_t imm;//const value in register
    struct Label label;
    int32_t pc;//offset of this instruction in bytes from start point
    int64_t address;
    Size_of_imm size_of_imm_data;
};



struct Data_CMDS
{
    struct Instruction *instructions;
    struct Label *labels_text;
    struct Label *labels_data;
    size_t size_of_instructions;
    size_t size_of_labels_text;
    size_t size_of_labels_data;
};

Errors_of_binary transform_asm_to_binary(FILE *file_pointer);

const struct CMD registers_array[] = {{"rax", 0x000},
                                {"rbx", 0x011},
                                {"rcx", 0x001},
                                {"rdx", 0x010},
                                {"eax", 0x000},
                                {"ebx", 0x011},
                                {"ecx", 0x001},
                                {"edx", 0x010}
                               };
const size_t size_of_registers_array = sizeof(registers_array) / sizeof(struct CMD);

// typedef struct {
//     Elf64_Word   sh_name;      // индекс имени секции в .shstrtab
//     Elf64_Word   sh_type;      // тип секции (например, PROGBITS, SYMTAB и т.п.)
//     Elf64_Xword  sh_flags;     // флаги секции (ALLOC, WRITE, EXECINSTR)
//     Elf64_Addr   sh_addr;      // адрес в памяти (не используется в ET_REL)
//     Elf64_Off    sh_offset;    // смещение в файле
//     Elf64_Xword  sh_size;      // размер секции
//     Elf64_Word   sh_link;      // ссылка (контекст-зависимо)
//     Elf64_Word   sh_info;      // дополнительная информация
//     Elf64_Xword  sh_addralign; // выравнивание
//     Elf64_Xword  sh_entsize;   // размер элемента, если секция — таблица
// } Elf64_Shdr;

// typedef struct {
//     Elf64_Word   st_name;   // индекс имени в .strtab
//     unsigned char st_info;  // тип и связывание
//     unsigned char st_other; // видимость
//     Elf64_Half   st_shndx;  // индекс секции
//     Elf64_Addr   st_value;  // адрес/смещение символа
//     Elf64_Xword  st_size;   // размер
// } Elf64_Sym;

// typedef struct {
//     unsigned char e_ident[EI_NIDENT]; // массив байтов для идентификации ELF-файла:
//                                        // - e_ident[0..3]: магическая метка ("0x7F 'E' 'L' 'F'")
//                                        // - далее: класс (32/64-бит), порядок байт (LE/BE), ABI, версия и т.п.
//                                        // Подробнее: e_ident включает индексы EI_MAG0..EI_CLASS..EI_DATA..EI_OSABI и т. д. :contentReference[oaicite:0]{index=0}

//     Elf64_Half e_type;      // тип ELF-файла: например, ET_REL (объектный), ET_EXEC (исполняемый), ET_DYN (разделяемая/динамическая) :contentReference[oaicite:1]{index=1}
//     Elf64_Half e_machine;   // целевая архитектура: например, EM_X86_64 для x86-64 :contentReference[oaicite:2]{index=2}
//     Elf64_Word e_version;   // версия ELF (обычно EV_CURRENT) :contentReference[oaicite:3]{index=3}

//     Elf64_Addr e_entry;     // виртуальный адрес точки входа (entry point); если нет — то 0 :contentReference[oaicite:4]{index=4}
//     Elf64_Off  e_phoff;     // смещение (off) программы header table в файле (или 0, если отсутствует) :contentReference[oaicite:5]{index=5}
//     Elf64_Off  e_shoff;     // смещение section header table в файле (или 0, если отсутствует) :contentReference[oaicite:6]{index=6}

//     Elf64_Word e_flags;     // процессор-зависимые флаги (обычно 0) :contentReference[oaicite:7]{index=7}

//     Elf64_Half e_ehsize;     // размер ELF-заголовка в байтах (обычно 64) :contentReference[oaicite:8]{index=8}
//     Elf64_Half e_phentsize;  // размер одной записи program header :contentReference[oaicite:9]{index=9}
//     Elf64_Half e_phnum;      // число записей в program header table :contentReference[oaicite:10]{index=10}
//     Elf64_Half e_shentsize;  // размер одной записи section header :contentReference[oaicite:11]{index=11}
//     Elf64_Half e_shnum;      // число записей в section header table :contentReference[oaicite:12]{index=12}
//     Elf64_Half e_shstrndx;   // индекс секции, содержащей имена всех секций (`.shstrtab`); если нет — SHN_UNDEF :contentReference[oaicite:13]{index=13}
// } Elf64_Ehdr;

#endif