/**
 * @file tixasm.h
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#ifndef TIXASM_H_
#define TIXASM_H_

#include <stdint.h>

#include "expr.h"
#include "reloc_table.h"
#include "symbol_table.h"

extern struct symbol_table *asm_symbol_table;
extern struct reloc_table *asm_reloc_table;

int asm_init(void);
void asm_destroy(void);

void asm_set_sec(enum section sec);
void asm_set_pc(uint16_t pc);
void asm_inc_pc(uint16_t off);

#endif /* TIXASM_H_ */

/* vim: set tw=80 ft=c: */
