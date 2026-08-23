#ifndef ASSEMBLE_H
#define SSEMBLE_H

// TODO: Document this file

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokens.h"

// TODO: Move this logic
// MACHINE CODE OPCODES (Phase 3: Binary generation)
#define O_TOKEN_MOVL 0xB8  ///< x86 opcode for "mov eax, imm32" (move 32-bit immediate to EAX)
#define O_TOKEN_RET  0xC3  ///< x86 opcode for "ret" (near return to calling procedure)


/// @brief Global index tracking the current byte position in the machine code buffer.
/// @note In a larger project, this should typically be declared as `extern` here 
///       and defined in exactly one `.c` file to avoid multiple definition errors.
int buffer_index = 0;

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

#endif