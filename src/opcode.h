/**
 * @file opcode.h
 * @author Zach Peltzer
 * @date Created: Fri, 02 Feb 2018
 * @date Last Modified: Mon, 05 Feb 2018
 */

#ifndef OPCODE_H_
#define OPCODE_H_

#include <stdio.h>
#include <stdint.h>

#include "expr.h"

/**
 * Maximum length of an instruction.
 * This is larger than any documented or undocumented instructions so that
 * user-defined compound instructions can be defined.
 */
#define INSTR_MAX_LEN 8

#define LIT_OFF_RST -2
#define LIT_OFF_IM  -3

/**
 * TODO Set values manually so that they align with token values and indirection
 * can be done by setting a flag.
 */
enum operand_type {
    OP_INVAL = -1,
    /** No operand */
    OP_NONE = 0,

    OP_IMM_START,

    /** Generic immediate type */
    OP_IMM,

    /** Immediate values */
    OP_IMM8, OP_IMM16,

    /** Restart address */
    OP_RST,

    /** Relative address */
    OP_REL,

    /** Bit index */
    OP_BIT,

    /** Interrupt mode */
    OP_IM,

    OP_IMM_END,

    OP_IMM_EXT_START,

    /** Extended address (dereferenced address) */
    OP_EXT,

    /** Port */
    OP_PORT,

    OP_IMM_EXT_END,

    /* 8-bit Registers */
    OP_A, OP_F, OP_B, OP_D, OP_E, OP_H, OP_L,
    OP_R, OP_I,
    OP_IXH, OP_IXL, OP_IYH, OP_IYL,

    /* For all intensive purposes, the register 'c' and the flag 'c' are the
     * same. The distinction is made by the instruction which matches with the
     * operands in what bytes it outupts.
     */
    OP_C,
    OP_fC = OP_C,

    /* 16-bit registers */
    OP_AF, OP_BC, OP_DE, OP_HL,
    OP_IX, OP_IY, OP_SP,

    /* 16-bit shadow registers */
    OP_sAF,

    /* Inderect registers */
    OP_iA, OP_iC, OP_iBC, OP_iDE, OP_iHL, OP_iSP,

    /* Inditect registers with index */
    OP_iIX, OP_iIY,

    /* Flags */
    OP_fNZ, OP_fZ, OP_fNC, OP_fPO, OP_fPE, OP_fP, OP_fM,
};

struct instruction {
    enum operand_type op1;
    enum operand_type op2;

    /**
     * Number of bytes in the instruction, not including any literals.
     */
    int size;

    /**
     * Offsets at which the values of the operands appear in the instruction.
     * This should always be -1 for operands which have no value.
     */
    int op1_off, op2_off;

    /**
     * The bytes of the instruction.
     */
    uint8_t bytes[INSTR_MAX_LEN];
};

/**
 * Stores the instructions associated with an opcode.
 */
struct opcode {
    /**
     * Opcode mnemonic.
     */
    const char *mnemonic;

    /**
     * Number of instructions.
     */
    int instr_count;

    /**
     * Registered instructions.
     * TODO Indirect this so that user instructions can be added.
     */
    const struct instruction *instrs;
};

struct operand {
    /**
     * Type of the operand.
     */
    enum operand_type type;

    /**
     * If the operand has a value, an expression representing that value.
     */
    struct expr_node *expr;
};

/**
 * Array of all registered opcodes.
 */
extern const struct opcode opcodes_builtin[];

enum operand_type op_type_indir(enum operand_type type);

/**
 * Frees the expression of an operand, if one exists.
 */
void operand_free_expr(struct operand *op);

const struct opcode *opcode_search(const char *mnemonic);

const struct instruction *opcode_match(const struct opcode *oc,
        const struct operand *op1, const struct operand *op2);

/**
 * Writes an instruction to a file stream.
 * This creates a new relocation entry if necessary (depending on the types of
 * the operands).
 */
int instr_output(const struct instruction *instr,
        const struct operand *op1, const struct operand *op2, FILE *stream);

#endif /* OPCODE_H_ */

/* vim: set tw=80 ft=c: */
