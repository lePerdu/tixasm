/**
 * @file main.c
 * @author Zach Peltzer
 * @date Created: Sat, 03 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include <stdio.h>


#include "opcode.h"
#include "tixasm.h"
#include "z80.tab.h"

extern FILE *yyin;
extern FILE *yyout;

int main(int argc, char *argv[]) {
    struct symbol_table st;
    struct reloc_table rt;

    char *output;
    size_t output_len;

    symtab_init(&st);
    reltab_init(&rt);
    asm_symbol_table = &st;
    asm_reloc_table = &rt;

    yyin = stdin;
    yyout = open_memstream(&output, &output_len);
    if (!yyout) {
        return -1;
    }

    yyparse();

    fclose(yyout);

    /* Perform relocations */
    for (int i = 0; i < reltab_get_size(&rt); i++) {
        const struct reloc_ent *ent = reltab_get(&rt, i);
        enum reloc_type type;
        int value, sym_value;

        if (!ent) {
            continue;
        }

        type = ent->type;
        if (type & RT_EXPR) {
            if (expr_eval(ent->expr) < 0) {
                fprintf(stderr, "Error, could not resolve expression.\n");
                continue;
            }

            switch (ent->expr->type) {
            case ET_CONST:
                sym_value = ent->expr->value;
                break;
            case ET_SYM:
                fprintf(stderr, "Could not resolve symbol.");
                continue;
            default:
                /* Should be unreachable */
                continue;
            }
        } else {
            sym_value = ent->sym->value;
        }

        value = ent->value;
        type &= type & ~RT_EXPR;
        /* Have to do processing before range checking */
        if (type == RT_REL_JUMP) {
            value = sym_value - value;
        } else {
            value = sym_value + value;
        }

        if (!reltab_in_range(type, value)) {
            fprintf(stderr, "Value %d out of range.\n", value);
            continue;
        }

        switch (type) {
        case RT_REL_JUMP:
        case RT_8_BIT:
        case RT_U_8_BIT:
        case RT_S_8_BIT:
            output[ent->offset] = value & 0xFF;
            break;
        case RT_16_BIT:
        case RT_U_16_BIT:
        case RT_S_16_BIT:
            output[ent->offset]   = value & 0xFF;
            output[ent->offset+1] = (value >> 8) & 0xFF;
            break;
        case RT_RST:
            output[ent->offset] |= value & 0xFF;
        case RT_IM:
            if (value == 0) {
                output[ent->offset] |= 0;
            } else if (value == 1) {
                output[ent->offset] |= 0x10;
            } else if (value == 2) {
                output[ent->offset] |= 0x18;
            }
            break;
        default:
            break;
        }
    }

    /* Print in hex format */
    for (int i = 0; i < output_len; i++) {
        printf("%02X ", output[i] & 0xFF);
    }
    printf("\n");

    free(output);
    symtab_destroy(&st);
    reltab_destroy(&rt);
    return 0;
}

/* vim: set tw=80 ft=c: */
