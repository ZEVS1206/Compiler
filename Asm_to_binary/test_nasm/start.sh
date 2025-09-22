nasm -f elf64 $1.asm -o $1.o
# ld -static $1.o -o $1 -lc -lm
# ./$1
