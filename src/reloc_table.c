/**
 * @file reltab.c
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include "reloc_table.h"

int reltab_in_range(enum reloc_type type, int value) {
    switch (type) {
    case RT_8_BIT:
        return 255 >= value && value >= -128;
    case RT_U_8_BIT:
        return 255 >= value && value >= 0;
    case RT_REL_JUMP:
        /* Once the relative offset is calculated, it should be a signed byte */
    case RT_S_8_BIT:
        return 127 >= value && value >= -128;
    case RT_16_BIT:
        return 65535 >= value && value >= -32768;
    case RT_U_16_BIT:
        return 65535 >= value && value >= 0;
    case RT_S_16_BIT:
        return 32767 >= value && value >= -32768;
    case RT_RST:
        /* Valid restart values are 0x00, 0x08, ..., 0x38, or
         * 0b00000000, 0b00001000, ..., 0b00111000 in binary. In other words,
         * only bits 3 - 5 can be set.
         */
        return !(value & ~0x38);
    case RT_IM:
        return value == 0 || value == 1 || value == 2;
    default:
        return 0;
    }
}

int reltab_init(struct reloc_table *rt) {
    return vector_init(&rt->relocs);
}

void reltab_destroy(struct reloc_table *rt){
    for (int i = 0; i < rt->relocs.size; i++) {
        struct reloc_ent * ent = vector_get(&rt->relocs, i);
        if (ent->type & RT_EXPR) {
            expr_free(ent->expr);
        }

        free(ent);
    }

    vector_destroy(&rt->relocs);
}

size_t reltab_get_size(const struct reloc_table *rt) {
    return rt ? rt->relocs.size : 0;
}

const struct reloc_ent *reltab_get(const struct reloc_table *rt, int idx) {
    return rt ? vector_get(&rt->relocs, idx) : NULL;
}

int reltab_remove(struct reloc_table *rt, int idx) {
    if (!rt) {
        return -1;
    }

    struct reloc_ent *ent = vector_get(&rt->relocs, idx);
    if (!ent) {
        return -1;
    }

    free(ent);
    return vector_remove(&rt->relocs, idx);
}

int reltab_add_sym(struct reloc_table *rt,
        enum reloc_type type, enum section sec, int offset, int value,
        const struct symbol_ent *symbol) {
    if (!rt) {
        return -1;
    }

    struct reloc_ent *ent = malloc(sizeof(*ent));
    if (!ent) {
        return -1;
    }

    if (vector_add(&rt->relocs, ent) < 0) {
        free(ent);
        return -1;
    }

    ent->type = type & ~RT_EXPR;
    ent->sec = sec;
    ent->offset = offset;
    ent->value = value;
    ent->sym = symbol;
    return 0;
}

int reltab_add_expr(struct reloc_table *rt,
        enum reloc_type type, enum section sec, int offset, int value,
        const struct expr_node *expr) {
    if (!rt) {
        return -1;
    }

    struct reloc_ent *ent = malloc(sizeof(*ent));
    if (!ent) {
        return -1;
    }

    if (vector_add(&rt->relocs, ent) < 0) {
        free(ent);
        return -1;
    }

    ent->type = type | RT_EXPR;
    ent->sec = sec;
    ent->offset = offset;
    ent->value = value;
    ent->expr = expr_clone(expr);
    return 0;
}


/* vim: set tw=80 ft=c: */
