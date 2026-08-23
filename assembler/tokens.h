#ifndef TOKENS_H
#define TOKENS_H

// TODO: Document this file


// ASSEMBLY LANGUAGE TOKENS (Phase 2: Assembly generation/parsing)
#define S_TOKEN_GLOBAL  ".global"     ///< Assembly directive to export a symbol
#define S_TOKEN_MAIN    COMPILER_TOKEN_MAIN  ///< Assembly label for the main function
#define S_TOKEN_MOVL    "movl"        ///< Move long (32-bit) instruction mnemonic
#define S_TOKEN_RET     "ret"         ///< Return instruction mnemonic

#endif