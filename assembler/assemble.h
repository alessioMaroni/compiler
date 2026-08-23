#ifndef ASSEMBLE_H
#define ASSEMBLE_H // Corretto il refuso (era SSEMBLE_H)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokens.h"

// ASSEMBLY LANGUAGE TOKENS (se non definiti in tokens.h)
#define S_TOKEN_ADDL "addl" ///< Assembly mnemonic for add instruction

// MACHINE CODE OPCODES (Phase 3: Binary generation)
#define O_TOKEN_MOVL 0xB8  ///< x86 opcode for "mov eax, imm32" (move 32-bit immediate to EAX)
#define O_TOKEN_ADDL 0x05  ///< x86 opcode for "add eax, imm32" (add 32-bit immediate to EAX)
#define O_TOKEN_RET  0xC3  ///< x86 opcode for "ret" (near return to calling procedure)

/// @brief Global index tracking the current byte position in the machine code buffer.
extern int buffer_index;

/// @brief Assembles an assembly mnemonic and its operand into raw machine code.
void assemble_s_token(unsigned char instruction_buffer[1024], const char* asm_mnemonic, const char* operand);

#endif // ASSEMBLE_H