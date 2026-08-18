#ifndef TOKENS_H
#define TOKENS_H

#include <string.h>
#include <stdio.h>

// -------------------C---------------------

#define C_TOKEN_INT       "int"
#define C_TOKEN_MAIN      "main"
#define C_TOKEN_RETURN    "return"

#define C_TOKEN_OPEN_ROUND_BRACKET  "("
#define C_TOKEN_CLOSE_ROUND_BRACKET ")"

#define C_TOKEN_OPEN_CURLY_BRACKET  "{"
#define C_TOKEN_CLOSE_CURLY_BRACKET "}"

// -------------------S---------------------

#define S_TOKEN_GLOBAL  ".global"
#define S_TOKEN_MAIN    C_TOKEN_MAIN
#define S_TOKEN_MOVL    "movl"
#define S_TOKEN_RET     "ret"

// -----------------------------------------

char* compile_token(FILE* asm_file_ptr, char* possible_token, int i){
    possible_token[i] = '\0';

    if(strcmp(possible_token, C_TOKEN_MAIN) == 0){
        fprintf(asm_file_ptr, "%s %s\n%s:\n", S_TOKEN_GLOBAL, S_TOKEN_MAIN, S_TOKEN_MAIN);
        return possible_token; 
    }
    
    if(strcmp(possible_token, C_TOKEN_RETURN) == 0){
        fprintf(asm_file_ptr, "    %s $0, %%eax\n    %s\n", S_TOKEN_MOVL, S_TOKEN_RET);
        return possible_token;
    }

    return NULL; 
}

#endif