FOLDER_ASSEMBLER = ./Assembler_nasm
FOLDER_READER = ./Reader

build_dir = BUILD
dir_asm = ../$(build_dir)/assembler.exe
dir_nasm = ../$(build_dir)/nasm_program.exe
file ?= test_file.rt2025

real_start_asm = $(build_dir)/./assembler.exe $(file)
start_asm = ./assembler.exe
start_nasm = $(build_dir)/./nasm_program.exe

ASM = asm
READ = rd

.PHONY: all clean run nasm assembler

all: assembler

assembler:
	mkdir -p $(build_dir)
	make -C $(FOLDER_ASSEMBLER) dir=$(dir_asm)

nasm:
	make -C $(FOLDER_ASSEMBLER) dir_nasm=$(dir_nasm) nasm

run: assembler
	$(real_start_asm)
	make -C $(FOLDER_ASSEMBLER) dir_nasm=$(dir_nasm) nasm
	$(start_nasm)

clean:
	$(MAKE) -C $(FOLDER_ASSEMBLER) clean
	rm -rf $(build_dir)
