#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// -------------------C---------------------
#define C_TOKEN_INT       "int"
#define C_TOKEN_MAIN      "main"
#define C_TOKEN_RETURN    "return"

// -------------------S---------------------
#define S_TOKEN_GLOBAL  ".global"
#define S_TOKEN_MAIN    C_TOKEN_MAIN
#define S_TOKEN_MOVL    "movl"
#define S_TOKEN_RET     "ret"

// -------------------O---------------------
#define O_TOKEN_MOVL 0xB8
#define O_TOKEN_RET  0xC3

//------------------------------------------

int buffer_index = 0;

char* compile_token(FILE* asm_file_ptr, char* possible_token, int i, FILE* c_file_ptr){
    possible_token[i] = '\0';

    if(strcmp(possible_token, C_TOKEN_MAIN) == 0){
        fprintf(asm_file_ptr, "%s %s\n%s:\n", S_TOKEN_GLOBAL, S_TOKEN_MAIN, S_TOKEN_MAIN);
        return possible_token; 
    }
    
    if(strcmp(possible_token, C_TOKEN_RETURN) == 0){
        char next_token[32];
        int idx = 0;
        int ch;

        while((ch = fgetc(c_file_ptr)) != EOF && (ch == ' ' || ch == '\t' || ch == '\n'));

        while(ch != EOF && ch != ' ' && ch != '\n' && ch != ';' && ch != '\t') {
            next_token[idx++] = (char)ch;
            ch = fgetc(c_file_ptr);
        }
        next_token[idx] = '\0';

        fprintf(asm_file_ptr, "    %s $%s, %%eax\n    %s\n", S_TOKEN_MOVL, next_token, S_TOKEN_RET);
        return possible_token;
    }

    return NULL; 
}

void assemble_s_token(unsigned char instruction_buffer[1024], const char* asm_mnemonic, const char* operand) {
    if(asm_mnemonic == NULL) return;

    if(strcmp(asm_mnemonic, S_TOKEN_MOVL) == 0 && operand != NULL) {
        int val = atoi(operand);

        if (buffer_index < 1024) instruction_buffer[buffer_index++] = O_TOKEN_MOVL;
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)(val & 0xFF);
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)((val >> 8) & 0xFF);
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)((val >> 16) & 0xFF);
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = (unsigned char)((val >> 24) & 0xFF);
    } 
    else if(strcmp(asm_mnemonic, S_TOKEN_RET) == 0) {
        if (buffer_index < 1024) instruction_buffer[buffer_index++] = O_TOKEN_RET;
    }
}

#endif