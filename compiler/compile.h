#ifndef COMPILE_H
#define COMPILE_H

// TODO: Document this file

#include <stdio.h>

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
char* compile_token(FILE* asm_file_ptr, char* possible_token, int i, FILE* c_file_ptr);

#endif