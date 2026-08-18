#include <stdio.h>
#include <string.h>

#include "tokens.h"

int check_file_ptr(FILE* file_ptr, const char* file_name) {
    if(file_ptr == NULL){
        printf("Error during %s opening\n", file_name);
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc < 2){
        printf("Error insufficient past data\n");
        return 1;
    }

    printf("\nCompiling...\n\n");

    char* c_file_name = argv[1];
    FILE* c_file_ptr = fopen(c_file_name, "r");
    if(check_file_ptr(c_file_ptr, c_file_name) == 1) return 1;

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
        
        while((x = fgetc(c_file_ptr)) != EOF){
            char ch = (char)x;
            printf("%c", ch);

            if(ch == ' ' || ch == '\n' || ch == '\t' || ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ';') {
                if (i > 0) {
                    token_found_buffer[i] = '\0';
                    
                    char* verified_token = compile_token(asm_file_ptr_w, token_found_buffer, i);
                    if (verified_token != NULL) {
                        printf("[TOKEN FOUND] Token: %s\n", verified_token);
                    }
                    
                    i = 0;
                }
            } else {
                if(i < sizeof(token_found_buffer) - 1) {
                    token_found_buffer[i] = ch;
                    i++;
                } else {
                    token_found_buffer[sizeof(token_found_buffer) - 1] = '\0';
                    i = 0;
                }
            }
        }
    }

    fclose(c_file_ptr);
    fclose(asm_file_ptr_w);

    FILE* asm_file_ptr_r = fopen(asm_file_name, "r");
    if(check_file_ptr(asm_file_ptr_r, asm_file_name) == 1) return 1;

    char object_file_name[64];
    snprintf(object_file_name, sizeof(object_file_name), "%s", c_file_name);
    object_file_name[strlen(object_file_name) - 1] = 'o'; 

    FILE* object_file_ptr = fopen(object_file_name, "w"); 
    if(check_file_ptr(object_file_ptr, object_file_name) == 1) {
        fclose(asm_file_ptr_r);
        return 1;
    }

    {
        int y = 0;
        int j = 0;
        while((y = fgetc(asm_file_ptr_r)) != EOF){
            printf("%c", (char)y);
            j++;
        }
    }

    fclose(asm_file_ptr_r);
    fclose(object_file_ptr);
    
    printf("\n\n");
    return 0;
}