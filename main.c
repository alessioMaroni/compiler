#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <elf.h>
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
        printf("Error: insufficient arguments provided\n");
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

            if(ch == ' ' || ch == '\n' || ch == '\t' || ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == ';') {
                if (i > 0) {
                    token_found_buffer[i] = '\0';
                    
                    compile_token(asm_file_ptr_w, token_found_buffer, i, c_file_ptr);
                    
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

    FILE* object_file_ptr = fopen(object_file_name, "wb"); 
    if(check_file_ptr(object_file_ptr, object_file_name) == 1) {
        fclose(asm_file_ptr_r);
        return 1;
    }

    unsigned char text_section[1024];

    {
        int y = 0;
        int j = 0;
        char token_found_buffer[32];
        
        extern int buffer_index;
        buffer_index = 0;

        while((y = fgetc(asm_file_ptr_r)) != EOF){
            char ch = (char)y;

            if(ch == ' ' || ch == '\n' || ch == '\t' || ch == ',' || ch == '$' || ch == ':') {
                if (j > 0) {
                    token_found_buffer[j] = '\0';
                    
                    if (strcmp(token_found_buffer, S_TOKEN_MOVL) == 0) {
                        char operand_buffer[32];
                        int op_idx = 0;
                        int next_ch;
                        
                        while((next_ch = fgetc(asm_file_ptr_r)) != EOF && (next_ch == ' ' || next_ch == '\t' || next_ch == '$' || next_ch == ','));
                        
                        while(next_ch != EOF && next_ch != ' ' && next_ch != '\n' && next_ch != '\t' && next_ch != ',') {
                            operand_buffer[op_idx++] = (char)next_ch;
                            next_ch = fgetc(asm_file_ptr_r);
                        }
                        operand_buffer[op_idx] = '\0';
                        
                        assemble_s_token(text_section, token_found_buffer, operand_buffer);
                    } else {
                        assemble_s_token(text_section, token_found_buffer, NULL);
                    }
                    
                    j = 0;
                }
            } else {
                if(j < sizeof(token_found_buffer) - 1) {
                    token_found_buffer[j] = ch;
                    j++;
                } else {
                    token_found_buffer[sizeof(token_found_buffer) - 1] = '\0';
                    j = 0;
                }
            }
        }

        Elf64_Ehdr ehdr = {0};
        ehdr.e_ident[EI_MAG0] = ELFMAG0;
        ehdr.e_ident[EI_MAG1] = ELFMAG1;
        ehdr.e_ident[EI_MAG2] = ELFMAG2;
        ehdr.e_ident[EI_MAG3] = ELFMAG3;
        ehdr.e_ident[EI_CLASS] = ELFCLASS64;
        ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
        ehdr.e_ident[EI_VERSION] = EV_CURRENT;
        ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;

        ehdr.e_type = ET_REL;
        ehdr.e_machine = EM_X86_64;
        ehdr.e_version = EV_CURRENT;
        ehdr.e_ehsize = sizeof(Elf64_Ehdr);
        ehdr.e_shentsize = sizeof(Elf64_Shdr);
        ehdr.e_shoff = sizeof(Elf64_Ehdr);
        ehdr.e_shnum = 2;

        fwrite(&ehdr, sizeof(Elf64_Ehdr), 1, object_file_ptr);

        Elf64_Shdr shdr = {0};
        shdr.sh_type = SHT_PROGBITS;
        shdr.sh_flags = SHF_ALLOC | SHF_EXECINSTR;
        shdr.sh_offset = sizeof(Elf64_Ehdr) + sizeof(Elf64_Shdr) * 2;
        shdr.sh_size = buffer_index;

        fwrite(&shdr, sizeof(Elf64_Shdr), 1, object_file_ptr);
        
        Elf64_Shdr null_shdr = {0};
        fwrite(&null_shdr, sizeof(Elf64_Shdr), 1, object_file_ptr);

        fwrite(text_section, 1, buffer_index, object_file_ptr);
    }

    fclose(asm_file_ptr_r);
    fclose(object_file_ptr);

    char exe_file_name[64];
    snprintf(exe_file_name, sizeof(exe_file_name), "%s", c_file_name);
    exe_file_name[strlen(exe_file_name) - 2] = '\0'; 

    FILE* out = fopen(exe_file_name, "wb");
    if (!out) {
        printf("Error creating executable file\n");
        return 1;
    }

    uint64_t base_addr = 0x400000;
    uint64_t headers_size = sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);

    unsigned char start_wrapper[] = {
        0xe8, 0x07, 0x00, 0x00, 0x00,
        0x89, 0xc7,
        0xb8, 0x3c, 0x00, 0x00, 0x00,
        0x0f, 0x05
    };

    uint64_t entry_point = base_addr + headers_size;
    uint64_t total_size = headers_size + sizeof(start_wrapper) + buffer_index;

    Elf64_Ehdr ehdr = {0};
    ehdr.e_ident[EI_MAG0] = ELFMAG0;
    ehdr.e_ident[EI_MAG1] = ELFMAG1;
    ehdr.e_ident[EI_MAG2] = ELFMAG2;
    ehdr.e_ident[EI_MAG3] = ELFMAG3;
    ehdr.e_ident[EI_CLASS] = ELFCLASS64;
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_ident[EI_VERSION] = EV_CURRENT;
    ehdr.e_ident[EI_OSABI] = ELFOSABI_NONE;

    ehdr.e_type = ET_EXEC;
    ehdr.e_machine = EM_X86_64;
    ehdr.e_version = EV_CURRENT;
    ehdr.e_entry = entry_point;
    ehdr.e_phoff = sizeof(Elf64_Ehdr);
    ehdr.e_ehsize = sizeof(Elf64_Ehdr);
    ehdr.e_phentsize = sizeof(Elf64_Phdr);
    ehdr.e_phnum = 1;

    Elf64_Phdr phdr = {0};
    phdr.p_type = PT_LOAD;
    phdr.p_flags = PF_R | PF_X;
    phdr.p_offset = 0;
    phdr.p_vaddr = base_addr;
    phdr.p_paddr = base_addr;
    phdr.p_filesz = total_size;
    phdr.p_memsz = total_size;
    phdr.p_align = 0x1000;

    fwrite(&ehdr, sizeof(Elf64_Ehdr), 1, out);
    fwrite(&phdr, sizeof(Elf64_Phdr), 1, out);
    fwrite(start_wrapper, sizeof(start_wrapper), 1, out);
    fwrite(text_section, 1, buffer_index, out);

    fclose(out);

    chmod(exe_file_name, 0755);
    
    printf("Compilation complete: executable generated successfully!\n\n");
    return 0;
}