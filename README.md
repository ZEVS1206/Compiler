# Compiler(for [My Language](https://github.com/ZEVS1206/MyLanguage))
*A custom compiler for my own programming language designed for education process and as for project for my portfolio.*  

---
## ⚠️⚠️⚠️Imporant fact
At the moment, the project is **still under active development**. Specifically, I am implementing my own nasm assembler compiler into a binary file that can be processed without problems by any system with the x86_64 instruction architecture. The main branch also contains a version that uses the **built-in** nasm compiler to get the binary!
Upon completion of the development of the custom compiler, the branches will be merged and the readme finalized.

---

## Introduction  
This project is the last part of the course of programming on the C and Assembler, which is taught by [Ilya Dedinsky](https://vk.com/ded32_ru) in the 1st course at MIPT. It brings together everything that was studied during all year of programming in C and Assembler. For me, the project became an **opportunity** to learn a lot of new information about the internal structure of compilers, feel the importance of a **fundamental understanding** of low-level programming and  gain invaluable experience.

**Key Goals**:  
- ✅ Goal 1: combine all the knowledge and skills received during the courses in C-language and Assembler.
- ✅ Goal 2: dive even deeper into the principles of programming languages and implement their work in a fairly simple way for my own language
- ✅ Goal 3: face the standard problems of such programs and overcome them to gain invaluable experience.

---

## Technical Specifications  
- **Compiler**: `g++` (GCC, version 15.2.0)  
- **IDE**: Microsoft VSCode
- **OS**: Kali Linux (version 2025.3)  
- **CPU**: Intel Core i7 13700H (2.4 GHz)  

**Build Requirements**:  
- C++17 or later  
- Makefile
- Nasm x86_64⚠️(Since the program was written specifically for this architecture, it **will not** work correctly on another one!)

---

## Development Tools  
The language was built using:  
- **Lexer/Parser**: *custom*
- **IR/Codegen**: *custom*
- **Compiler of nasm code**: *nasm*
- **Debugging**: "GDB", "Valgrind"

---

## More information about the project

All information on the "frontend" and supported functionality of my language can be found in a separate [repository](https://github.com/ZEVS1206/MyLanguage).
Here I will focus more on the details of the **backend implementation**.

### Compilation Stages
As you know, there are **4 main stages** of compilation:

#### 1. Preprocessing
At this stage, macros are substituted, memory is allocated for static arrays (if any), and the program is prepared for compilation(syntax errors are being checked).

#### 2. Compilation
After the program has passed the first stage in a "high-level" language, it is translated into assembly code. At this stage, the main errors related to more complex syntax, working with *pointers* and *memory* are tracked. We can also say that at this stage, the main **optimizations** that the compiler makes occur.

#### 3. Assembly
At this stage, the assembler program is translated into **binary code**, and an **object file** is created with all the necessary *headers*, *section placements*, and *machine instruction codes*. However, at this stage, **nothing** is yet known about the positions of transitions to labels, function call addresses, and also about the toolkit of **external libraries**.

#### 4. Linking
Finally, the linker calculates all the necessary addresses and offsets, connects external libraries, and places the program in real memory instead of virtual memory. The output is an **executable file**, which is already running when the program is started.

### What is about my compiler?
In principle, all the steps were implemented in [my own language](https://github.com/ZEVS1206/MyLanguage), in fact, it was compiled and processed by a separate independent program (**virtual machine**). However, now I want to use more **advanced and serious tools** to complete these phases.
Since the first phase is implemented at the stage of the parser and handler, we will proceed immediately to the second stage. And so specifically in **this branch** of the project, a version has been implemented in which the first **2 stages** of compilation are performed entirely by my **independent programs**.
In the future, it is planned to complete the development of a program that performs **phase 3**! 
However, I do not plan to implement my linking stage. The fact is that this is a very subtle and important stage, and it is better to use ready-made proven solutions to reduce the number of problems).

### Features of the compilation stage implementation
First of all, I used **nasm assembly** language compilation, since it is one of the most popular on Linux systems.

Since it was assumed that all the functions that are **built** into the language would be implemented somehow internally, they were actually implemented in assembly language. These functions are `print`, `input`, `int`, `double`. All implementations of auxiliary and main functions are located in the ``Assembler_nasm/include/asm_built_in_functions.inc``

Example:
```nasm
%macro int_to_double 2
    cvtsi2sd xmm%1, %2
%endmacro

%macro double_to_int 2
    cvtsd2si %1, xmm%2
%endmacro

%macro floor_double 2
    cvttsd2si %1, xmm%2
%endmacro

%macro input_str 2
    mov rax, 0
    mov rdi, 0
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro

%macro print_str 2
    mov rax, 1
    mov rdi, 1
    mov rsi, %1
    mov rdx, %2
    syscall
%endmacro
```

I note that like the **previous version**, this version **fully supports** working with **fractional numbers**!

### Additional functionality
Since the exponentiation function is not implemented in the standard nasm, its version from the external **libm** library is used. Thus, the connection of this library is used in the linking parameters.
Here are the compilation and linking options for the generated **nasm file**:
```bash
nasm -f elf64 $(NASM_SOURCE_DIR)/*.asm -o $(NASM_OBJECTS)
ld $(NASM_OBJECTS) -o $(dir_nasm) -lc -lm --dynamic-linker /lib64/ld-linux-x86-64.so.2
```

### Working with fractional numbers
I'll say a little bit about this implementation detail. The method was used by placing numbers on labels, and then accessing them through the usual **rel addressing**.

Example:
```nasm
movsd xmm0, [rel LC0]
;........
LC0:
	dq 0.500000
```

### About name of labels

To ensure the strict uniqueness of labels, including those associated with conditional if/else statements, all names are generated along with the address of the **node** of the program tree.

### Global and local scope
When using functions, support for **local and global variables** is implemented (any variables that **are not included in any function** are considered **global**).

---
## ⚙️ Build and Run

The first thing to do is download the repository!

```bash
git clone https://github.com/ZEVS1206/Compiler.git
cd Compiler
```

### Simple start

Then, if you just want to use it for execute some programm, you need to create file with extension ``.rt2025``(or use default ``test_file.rt2025``) and write your program into the file.

Then, just build:
```bash
make clean
make
```
And run
```bash
make run file={your_file_name}.rt2025
```
### Research
If you want to study the generated asm file without further assembling, linking and running it, you can execute:
```bash
make assembler
make asm_run
```
Then, in the file ``Assembler_nasm/source/nasm/asm_programm.asm`` you can see generated code.
For execute it, you need:
```bash
make nasm
make nasm_run
```
---

## Conclusion  
This project has given me a lot of skills and knowledge. I am very grateful to [Ilya Rudolfovich Dedinsky](https://github.com/ded32?tab=overview&from=2025-07-01&to=2025-07-22) and my mentor [Kolya Kasparov](https://github.com/nniikon) for sharing their invaluable experience.

---
## 📄 License

This project is licensed under the **MIT License**.

---

## 👨‍💻 Author
Created by [Zevs](https://github.com/ZEVS1206)
