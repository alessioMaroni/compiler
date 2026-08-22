/// @file tokens.h
/// @brief Token definitions and core parsing/assembly functions for the custom compiler.
///
/// This header file contains the definitions for the lexical tokens recognized
/// during the C-to-Assembly phase, the Assembly-to-Machine-Code phase, and the
/// direct machine opcodes. It also implements the logic to translate C tokens 
/// into x86_64 assembly, and assembly mnemonics into raw binary machine code.

#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// C LANGUAGE TOKENS
#define C_TOKEN_INT       "int"       ///< Integer type keyword
#define C_TOKEN_MAIN      "main"      ///< Main entry point identifier
#define C_TOKEN_RETURN    "return"    ///< Return statement keyword

// ASSEMBLY LANGUAGE TOKENS (Phase 2: Assembly generation/parsing)
#define S_TOKEN_GLOBAL  ".global"     ///< Assembly directive to export a symbol
#define S_TOKEN_MAIN    C_TOKEN_MAIN  ///< Assembly label for the main function
#define S_TOKEN_MOVL    "movl"        ///< Move long (32-bit) instruction mnemonic
#define S_TOKEN_RET     "ret"         ///< Return instruction mnemonic

// MACHINE CODE OPCODES (Phase 3: Binary generation)
#define O_TOKEN_MOVL 0xB8  ///< x86 opcode for "mov eax, imm32" (move 32-bit immediate to EAX)
#define O_TOKEN_RET  0xC3  ///< x86 opcode for "ret" (near return to calling procedure)

/// @brief Global index tracking the current byte position in the machine code buffer.
/// @note In a larger project, this should typically be declared as `extern` here 
///       and defined in exactly one `.c` file to avoid multiple definition errors.
int buffer_index = 0;

/// @brief Compiles a recognized C token into its corresponding Assembly instructions.
///
/// This function analyzes a parsed token from the source C file. If the token
/// matches known constructs (like `main` or `return`), it writes the equivalent 
/// x86_64 assembly instructions directly to the output `.s` file.
///
/// @param asm_file_ptr   File pointer to the output Assembly (`.s`) file.
/// @param possible_token The current string token extracted from the C source.
/// @param i              The length of the current token.
/// @param c_file_ptr     File pointer to the source C file (used to look ahead for operands, 
///                       e.g., reading the return value after a "return" keyword).
///
/// @return Returns the matched token string if translation occurred, otherwise returns NULL.
char* compile_token(FILE* asm_file_ptr, char* possible_token, int i, FILE* c_file_ptr) {
    // Ensure the token string is null-terminated
    possible_token[i] = '\0';

    // Handle "main" token: generate the global entry point
    if(strcmp(possible_token, C_TOKEN_MAIN) == 0){
        // Writes: 
        // .global main
        // main:
        fprintf(asm_file_ptr, "%s %s\n%s:\n", S_TOKEN_GLOBAL, S_TOKEN_MAIN, S_TOKEN_MAIN);
        return possible_token; 
    }
    
    // Handle "return" token: look ahead for the return value and generate movl + ret
    if(strcmp(possible_token, C_TOKEN_RETURN) == 0){
        char next_token[32];
        int idx = 0;
        int ch;

        // Skip whitespace characters to find the return value (operand)
        while((ch = fgetc(c_file_ptr)) != EOF && (ch == ' ' || ch == '\t' || ch == '\n'));

        // Extract the operand (e.g., the number to return)
        while(ch != EOF && ch != ' ' && ch != '\n' && ch != ';' && ch != '\t') {
            next_token[idx++] = (char)ch;
            ch = fgetc(c_file_ptr);
        }
        next_token[idx] = '\0'; // Null-terminate the operand string

        // Writes:
        //     movl $VALUE, %eax
        //     ret
        fprintf(asm_file_ptr, "    %s $%s, %%eax\n    %s\n", S_TOKEN_MOVL, next_token, S_TOKEN_RET);
        return possible_token;
    }

    return NULL; // Token was not recognized as a compilable keyword
}

/// @brief Assembles an assembly mnemonic and its operand into raw machine code.
///
/// This function acts as a mini-assembler. It takes an assembly instruction 
/// mnemonic (e.g., `movl` or `ret`) and translates it into standard x86_64 
/// machine code bytes, storing them sequentially in the provided instruction buffer.
///
/// @param instruction_buffer The byte array where the compiled machine code will be written.
/// @param asm_mnemonic       The assembly instruction parsed from the `.s` file.
/// @param operand            The operand string (if any) associated with the instruction.
void assemble_s_token(unsigned char instruction_buffer[1024], const char* asm_mnemonic, const char* operand) {
    if(asm_mnemonic == NULL) return;

    // Handle 'movl' instruction (Move Immediate 32-bit to EAX)
    if(strcmp(asm_mnemonic, S_TOKEN_MOVL) == 0 && operand != NULL) {
        // Convert the string operand to an integer
        int val = atoi(operand);

        // Write the opcode (0xB8) to the buffer
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = O_TOKEN_MOVL;
        
        // Write the 32-bit integer in Little-Endian byte order (x86 standard)
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)(val & 0xFF);         // Least significant byte
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)((val >> 8) & 0xFF);
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)((val >> 16) & 0xFF);
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)((val >> 24) & 0xFF); // Most significant byte
    } 
    // Handle 'ret' instruction (Return)
    else if(strcmp(asm_mnemonic, S_TOKEN_RET) == 0) {
        // Write the opcode (0xC3) to the buffer
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = O_TOKEN_RET;
    }
}

#endif // TOKENS_H