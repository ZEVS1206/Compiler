CC=g++
#CFLAGS=-Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE
CFLAGS=-ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr
SOURCE_DIR_ASM = source
SOURCE_DIR_READER = ../Reader/source
BUILD_DIR = build
DUMP_DIR = dump
HEADERS_DIR_ASM = include
INCLUDE_ASM= -I $(HEADERS_DIR_ASM)
CFLAGS+=$(INCLUDE_ASM)
HEADERS_DIR_READER = ../Reader/include
INCLUDE_READER = -I $(HEADERS_DIR_READER)
CFLAGS+=$(INCLUDE_READER)
HEADERS=$(wildcard $(HEADERS_DIR_ASM)/*.h)
SOURCES_ASM=$(wildcard $(SOURCE_DIR_ASM)/*.cpp)
SOURCES_READER=$(wildcard $(SOURCE_DIR_READER)/*.cpp)
OBJECTS_ASM=$(patsubst $(SOURCE_DIR_ASM)/%.cpp,$(BUILD_DIR)/asm_%.o,$(SOURCES_ASM))
OBJECTS_READER=$(patsubst $(SOURCE_DIR_READER)/%.cpp,$(BUILD_DIR)/reader_%.o,$(SOURCES_READER))
OBJECTS=$(OBJECTS_ASM) $(OBJECTS_READER)
EXEC_NAME := assembler.exe

dir ?= $(BUILD_DIR)/$(EXEC_NAME)


# wildcart patsubst
.PHONY: clean all run

all: $(dir)

$(dir): $(OBJECTS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/asm_%.o: $(SOURCE_DIR_ASM)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

$(BUILD_DIR)/reader_%.o: $(SOURCE_DIR_READER)/%.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -c -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(DUMP_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DUMP_DIR)

run:
	$(dir)
