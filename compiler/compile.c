#include <stdio.h>

#include "compile.h"
#include "tokens/data_types.h"
#include "tokens/arithmetic_operators.h"
#include "tokens/basic.h"

#include <string.h>

#include "../assembler/tokens.h"

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
    if(strcmp(possible_token, COMPILER_TOKEN_MAIN) == 0){
        // Writes: 
        // .global main
        // main:
        fprintf(asm_file_ptr, "%s %s\n%s:\n", S_TOKEN_GLOBAL, S_TOKEN_MAIN, S_TOKEN_MAIN);
        return possible_token; 
    }
    
    // Handle "return" token: look ahead for the return value and generate movl + ret
    if(strcmp(possible_token, COMPILER_TOKEN_RETURN) == 0){
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
