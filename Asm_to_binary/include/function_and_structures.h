#ifndef FUNCTION_AND_STRUCTURES_H
#define FUNCTION_AND_STRUCTURES_H
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
    ERROR_OF_READING_CMDS            = 7
};

struct CMD
{
    char original_name[50];
    uint8_t byte_code[16];
};

struct Binary_file
{
    FILE *file_pointer;
    char section_header_str_table[256];//Table of names of sections
    size_t len_section_header_str_table;
    char symbol_string_table[256];//Table of names of symbols
    size_t len_symbol_string_table;
    Elf64_Ehdr ELF_Header;
    uint8_t *buffer_of_commands;
    size_t len_buffer_of_commands;
};

Errors_of_binary transform_asm_to_binary(FILE *file_pointer);


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