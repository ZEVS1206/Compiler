FOLDER_ASSEMBLER = ./Assembler_nasm
FOLDER_READER = ./Reader

dir_asm = ../assembler.exe
dir_nasm = ../nasm_program.exe
file ?= test_file.rt2025

real_start_asm = ./assembler.exe $(file)
start_asm = ./assembler.exe
start_nasm = ./nasm_program.exe

ASM = asm
READ = rd

.PHONY: all clean run nasm assembler

all: assembler

assembler:
	make -C $(FOLDER_ASSEMBLER) dir=$(dir_asm)

nasm:
	make -C $(FOLDER_ASSEMBLER) dir_nasm=$(dir_nasm) nasm

run: assembler
	$(real_start_asm)
	make -C $(FOLDER_ASSEMBLER) dir_nasm=$(dir_nasm) nasm
	$(start_nasm)

clean:
	$(MAKE) -C $(FOLDER_ASSEMBLER) clean
	rm -rf $(start_nasm)
	rm -rf $(start_asm)
