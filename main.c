/// @file main.c
/// @brief Monolithic custom compiler, assembler, and ELF64 binary generator.
///
/// This toolchain takes a custom C-like source file, parses it into an intermediate
/// assembly representation (.s), assembles that into machine code, and packages it 
/// into both a standard ELF object file (.o) and a standalone freestanding executable.
/// It works entirely in memory for the machine code generation and bypasses external 
/// linkers to build the final binary.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <elf.h>
#include "tokens.h"

/// @brief Helper function that checks if the value of the file pointer is different from NULL.
///
/// This function validates whether a given file pointer has been successfully initialized (i.e., is not NULL). 
/// If the pointer is NULL, it logs an error message containing the filename and returns an error code.
///
/// @param file_ptr The `FILE*` pointer that we want to check for validity.
/// @param file_name A string representing the name of the file, used solely for error reporting 
///                  when printing diagnostic messages.
///
/// @return Returns `0` if the file pointer is valid (not NULL). 
///         Returns `1` if the file pointer is NULL (indicating an opening or initialization failure).
int check_file_ptr(FILE* file_ptr, const char* file_name) {
    if (file_ptr == NULL) {
        // TODO: Remove this printf in favor of a centralized error-handling mechanism
        printf("Error during %s opening\n", file_name);
        return 1;
    }
    return 0;
}

/// @brief Main execution entry point for the custom compiler toolchain.
///
/// This function coordinates the four major phases of the compilation process:
/// 1. Tokenization and intermediate Assembly Generation (.s).
/// 2. Machine Code Assembly from the generated .s file.
/// 3. Object File Generation (.o) following the ELF64 standard format.
/// 4. Executable Generation (freestanding ELF binary with an execution wrapper).
///
/// @param argc Argument count. Expects at least 2 (the program name and the target source file).
/// @param argv Argument vector. argv[1] should be the path to the .c source file.
///
/// @return Returns `0` on successful compilation, assembly, and linking. 
///         Returns `1` if an error occurs during file I/O or processing.
int main(int argc, char* argv[]) {
    // Ensure the user provided a source file to compile
    if(argc < 2){
        printf("Error: insufficient arguments provided\n");
        return 1;
    }

    printf("\nCompiling...\n\n");

    // PHASE 1: COMPILATION (C to Assembly)

    char* c_file_name = argv[1];
    FILE* c_file_ptr = fopen(c_file_name, "r");
    if(check_file_ptr(c_file_ptr, c_file_name) == 1) return 1;

    // Dynamically create the assembly file name by replacing the last char ('c') with 's'
    char asm_file_name[64];
    snprintf(asm_file_name, sizeof(asm_file_name), "%s", c_file_name);
    asm_file_name[strlen(asm_file_name) - 1] = 's';

    FILE* asm_file_ptr_w = fopen(asm_file_name, "w");
    if(check_file_ptr(asm_file_ptr_w, asm_file_name) == 1) {
        fclose(c_file_ptr);
        return 1;
    }

    {
        int x = 0;
        int i = 0;
        char token_found_buffer[32];
        
        // Basic lexer: Read character by character from the source file
        while((x = fgetc(c_file_ptr)) != EOF){
            char ch = (char)x;

            // Delimiters that mark the end of a token
            if(ch == ' ' || ch == '\n' || ch == '\t' || ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ';') {
                if (i > 0) {
                    token_found_buffer[i] = '\0'; // Null-terminate the extracted token
                    
                    // Pass the recognized token to the compiler logic to generate assembly
                    compile_token(asm_file_ptr_w, token_found_buffer, i, c_file_ptr);
                    
                    i = 0; // Reset token buffer index
                }
            } else {
                // Accumulate characters into the current token buffer safely
                if(i < sizeof(token_found_buffer) - 1) {
                    token_found_buffer[i] = ch;
                    i++;
                } else {
                    // Prevent buffer overflow by truncating overly long tokens
                    token_found_buffer[sizeof(token_found_buffer) - 1] = '\0';
                    i = 0;
                }
            }
        }
    }

    fclose(c_file_ptr);
    fclose(asm_file_ptr_w);

    // PHASE 2: ASSEMBLY (Assembly to Machine Code)

    FILE* asm_file_ptr_r = fopen(asm_file_name, "r");
    if(check_file_ptr(asm_file_ptr_r, asm_file_name) == 1) return 1;

    // Create the object file name (.o)
    char object_file_name[64];
    snprintf(object_file_name, sizeof(object_file_name), "%s", c_file_name);
    object_file_name[strlen(object_file_name) - 1] = 'o'; 

    FILE* object_file_ptr = fopen(object_file_name, "wb"); 
    if(check_file_ptr(object_file_ptr, object_file_name) == 1) {
        fclose(asm_file_ptr_r);
        return 1;
    }

    unsigned char text_section[1024]; // Buffer holding the raw machine code instructions

    {
        int y = 0;
        int j = 0;
        char token_found_buffer[32];
        
        // Index tracking the current byte position in the text_section buffer
        extern int buffer_index;
        buffer_index = 0;

        // Assembly lexer loop
        while((y = fgetc(asm_file_ptr_r)) != EOF){
            char ch = (char)y;

            // Assembly token delimiters
            if(ch == ' ' || ch == '\n' || ch == '\t' || ch == ',' || ch == '$' || ch == ':') {
                if (j > 0) {
                    token_found_buffer[j] = '\0';
                    
                    // Special handling for instructions with operands (e.g., 'movl')
                    if (strcmp(token_found_buffer, S_TOKEN_MOVL) == 0) {
                        char operand_buffer[32];
                        int op_idx = 0;
                        int next_ch;
                        
                        // Skip whitespace and formatting characters to find the operand
                        while((next_ch = fgetc(asm_file_ptr_r)) != EOF && (next_ch == ' ' || next_ch == '\t' || next_ch == '$' || next_ch == ','));
                        
                        // Extract the operand string
                        while(next_ch != EOF && next_ch != ' ' && next_ch != '\n' && next_ch != '\t' && next_ch != ',') {
                            operand_buffer[op_idx++] = (char)next_ch;
                            next_ch = fgetc(asm_file_ptr_r);
                        }
                        operand_buffer[op_idx] = '\0';
                        
                        // Translate to machine code with operand
                        assemble_s_token(text_section, token_found_buffer, operand_buffer);
                    } else {
                        // Translate to machine code without operand
                        assemble_s_token(text_section, token_found_buffer, NULL);
                    }
                    
                    j = 0;
                }
            } else {
                // Accumulate chars for assembly tokens safely
                if(j < sizeof(token_found_buffer) - 1) {
                    token_found_buffer[j] = ch;
                    j++;
                } else {
                    token_found_buffer[sizeof(token_found_buffer) - 1] = '\0';
                    j = 0;
                }
            }
        }

        // PHASE 3: OBJECT FILE WRITING (ELF64 ET_REL)
        
        // Setup the ELF64 Header for a relocatable object file (.o)
        Elf64_Ehdr ehdr = {0};
        ehdr.e_ident[EI_MAG0] = ELFMAG0;        // 0x7f
        ehdr.e_ident[EI_MAG1] = ELFMAG1;        // 'E'
        ehdr.e_ident[EI_MAG2] = ELFMAG2;        // 'L'
        ehdr.e_ident[EI_MAG3] = ELFMAG3;        // 'F'
        ehdr.e_ident[EI_CLASS] = ELFCLASS64;    // 64-bit architecture
        ehdr.e_ident[EI_DATA] = ELFDATA2LSB;    // Little Endian
        ehdr.e_ident[EI_VERSION] = EV_CURRENT;  // Current ELF version
        ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE; // System V ABI

        ehdr.e_type = ET_REL;                   // Relocatable file type
        ehdr.e_machine = EM_X86_64;             // AMD64 / x86_64 Instruction set
        ehdr.e_version = EV_CURRENT;
        ehdr.e_ehsize = sizeof(Elf64_Ehdr);     // Size of the ELF header
        ehdr.e_shentsize = sizeof(Elf64_Shdr);  // Size of a Section Header
        ehdr.e_shoff = sizeof(Elf64_Ehdr);      // Section Headers follow immediately after ELF Header
        ehdr.e_shnum = 2;                       // Number of section headers (NULL and .text)

        fwrite(&ehdr, sizeof(Elf64_Ehdr), 1, object_file_ptr);

        // Standard ELF convention: Section 0 is always the NULL section
        Elf64_Shdr null_shdr = {0};
        fwrite(&null_shdr, sizeof(Elf64_Shdr), 1, object_file_ptr);

        // Section 1: .text section containing our executable machine code
        Elf64_Shdr shdr = {0};
        shdr.sh_type = SHT_PROGBITS;            // Program data
        shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR; // Allocate in memory, Executable code
        shdr.sh_offset = sizeof(Elf64_Ehdr) + (sizeof(Elf64_Shdr) * 2); // Data starts after the headers
        shdr.sh_size = buffer_index;            // Size of our generated machine code

        fwrite(&shdr, sizeof(Elf64_Shdr), 1, object_file_ptr);

        // Write the actual compiled machine code byte array into the file
        fwrite(text_section, 1, buffer_index, object_file_ptr);
    }

    fclose(asm_file_ptr_r);
    fclose(object_file_ptr);

    // PHASE 4: STANDALONE EXECUTABLE GENERATION (ELF64 ET_EXEC)
    
    // Create the final binary name by truncating the file extension
    char exe_file_name[64];
    snprintf(exe_file_name, sizeof(exe_file_name), "%s", c_file_name);
    exe_file_name[strlen(exe_file_name) - 2] = '\0'; 

    FILE* out = fopen(exe_file_name, "wb");
    if (!out) {
        printf("Error creating executable file\n");
        return 1;
    }

    // Standard starting virtual address for x86_64 ELF executables
    uint64_t base_addr = 0x400000;
    uint64_t headers_size = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);

    // Wrapper stub: sets up environment, calls our code, and securely exits.
    // e8 09 00 00 00 -> call function (jumps over the exit syscall to our code)
    // 89 c7          -> mov edi, eax (takes return value and puts it in exit arg)
    // b8 3c 00 00 00 -> mov eax, 60 (sys_exit)
    // 0f 05          -> syscall
    unsigned char start_wrapper[] = {
        0xe8, 0x09, 0x00, 0x00, 0x00,
        0x89, 0xc7,
        0xb8, 0x3c, 0x00, 0x00, 0x00,
        0x0f, 0x05
    };

    // Calculate memory layout sizes
    uint64_t entry_point = base_addr + headers_size;
    uint64_t total_size = headers_size + sizeof(start_wrapper) + buffer_index;

    // Set up ELF Header for an Executable File
    Elf64_Ehdr ehdr = {0};
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;

    ehdr.e_type = ET_EXEC;                  // Standalone Executable
    ehdr.e_machine = EM_X86_64;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = entry_point;             // Program entry point (starts at our wrapper)
    ehdr.e_phoff = sizeof(Elf64_Ehdr);      // Program Headers immediately follow ELF Header
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = 1;                       // Just one Program Header needed for memory loading

    // Setup the Program Header (tells the OS loader how to map the file into memory)
    Elf64_Phdr phdr = {0};
    phdr.p_type = PT_LOAD;                  // Loadable segment
    phdr.p_flags = PF_R | PF_X;             // Read and Execute permissions
    phdr.p_offset = 0;                      // Map starting from the very beginning of the file
    phdr.p_vaddr = base_addr;               // Virtual address in memory
    phdr.p_paddr = base_addr;               // Physical address (usually matches virtual in modern OS)
    phdr.p_filesz = total_size;             // Size of the segment in the file
    phdr.p_memsz = total_size;              // Size of the segment in memory
    phdr.p_align = 0x1000;                  // Align to standard 4KB page bounds

    // Construct the executable file
    fwrite(&ehdr, sizeof(Elf64_Ehdr), 1, out);          // Write ELF Header
    fwrite(&phdr, sizeof(Elf64_Phdr), 1, out);          // Write Program Header
    fwrite(start_wrapper, sizeof(start_wrapper), 1, out); // Write execution wrapper
    fwrite(text_section, 1, buffer_index, out);         // Write the actual compiled code

    fclose(out);

    // Make the generated binary executable by the user
    chmod(exe_file_name, 0755);
    
    printf("Compilation complete: executable generated successfully!\n\n");
    return 0;
}