/**
 * @file tixasm.c
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include "tixasm.h"

/**
 * Different program counters are used for each section. Switching sections just
 * switches which one of these asm_pc points to.
 */
static struct expr_node *asm_text_pc = NULL,
                        *asm_data_pc = NULL,
                        *asm_abs_pc  = NULL;

static struct expr_node **asm_pc = NULL;

struct symbol_table *asm_symbol_table = NULL;
struct reloc_table *asm_reloc_table = NULL;

int asm_init(void) {
    asm_text_pc = expr_alloc_const(SEC_TEXT, 0);
    if (!asm_text_pc) {
        goto INIT_FAIL;
    }

    asm_data_pc = expr_alloc_const(SEC_DATA, 0);
    if (!asm_data_pc) {
        goto INIT_FAIL;
    }

    asm_abs_pc = expr_alloc_const(SEC_ABS, 0);
    if (!asm_abs_pc) {
        goto INIT_FAIL;
    }

    asm_symbol_table = malloc(sizeof(*asm_symbol_table));
    if (!asm_symbol_table || symtab_init(asm_symbol_table) < 0) {
        goto INIT_FAIL;
    }

    asm_reloc_table = malloc(sizeof(*asm_reloc_table));
    if (!asm_reloc_table || reltab_init(asm_reloc_table) < 0) {
        goto INIT_FAIL;
    }

    asm_pc = &asm_abs_pc;
    return 0;

INIT_FAIL:
    asm_destroy();
    return -1;
}

void asm_destroy(void) {
    expr_free(asm_text_pc);
    expr_free(asm_data_pc);
    expr_free(asm_abs_pc);

    symtab_destroy(asm_symbol_table);
    free(asm_symbol_table);

    reltab_destroy(asm_reloc_table);
    free(asm_reloc_table);

    asm_pc = NULL;
}

void asm_set_sec(enum section sec) {
    switch (sec) {
    case SEC_TEXT:
        asm_pc = &asm_text_pc;
        break;
    case SEC_DATA:
        asm_pc = &asm_data_pc;
        break;
    case SEC_ABS:
        asm_pc = &asm_abs_pc;
        break;
    default:
        break;
    }
}

void asm_set_pc(uint16_t pc) {
    enum section sec = (*asm_pc)->sec;
    expr_free(*asm_pc);
    *asm_pc = expr_alloc_const(sec, pc);
}

void asm_set_pc_expr(const struct expr_node *pc) {
    if (!pc) {
        return;
    }

    expr_free(*asm_pc);
    *asm_pc = expr_clone(pc);
}

void asm_inc_pc(uint16_t off) {
    *asm_pc = expr_alloc('+', *asm_pc, expr_alloc_const(SEC_ABS, off));
}

/* vim: set tw=80 ft=c: */
