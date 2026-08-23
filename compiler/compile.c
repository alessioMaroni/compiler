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
    
    // TODO: Use c tokens and asm tokens
    // Handle "return" token: look ahead for the return value and generate movl + ret
    if (strcmp(possible_token, COMPILER_TOKEN_RETURN) == 0) {
        char op1[32] = {0};
        char op2[32] = {0};
        int idx1 = 0, idx2 = 0;
        int has_plus = 0;
        int ch;

        // Skip leading whitespace after "return"
        while ((ch = fgetc(c_file_ptr)) != EOF && (ch == ' ' || ch == '\t' || ch == '\n'));

        // Parse expression characters until the semicolon ';'
        while (ch != EOF && ch != ';') {
            if (ch == '+') {
                has_plus = 1;
            } else if (ch != ' ' && ch != '\t' && ch != '\n') {
                if (!has_plus) {
                    if (idx1 < sizeof(op1) - 1) op1[idx1++] = (char)ch;
                } else {
                    if (idx2 < sizeof(op2) - 1) op2[idx2++] = (char)ch;
                }
            }
            ch = fgetc(c_file_ptr);
        }
        op1[idx1] = '\0';
        op2[idx2] = '\0';

        if (has_plus) {
            // Emits:
            //     movl $OP1, %eax
            //     addl $OP2, %eax
            //     ret
            fprintf(asm_file_ptr, "    %s $%s, %%eax\n", S_TOKEN_MOVL, op1);
            fprintf(asm_file_ptr, "    addl $%s, %%eax\n", op2);
            fprintf(asm_file_ptr, "    %s\n", S_TOKEN_RET);
        } else {
            // Emits standard return:
            //     movl $OP1, %eax
            //     ret
            fprintf(asm_file_ptr, "    %s $%s, %%eax\n    %s\n", S_TOKEN_MOVL, op1, S_TOKEN_RET);
        }

        return possible_token;
    }
    
    return NULL; // Token was not recognized as a compilable keyword
}
