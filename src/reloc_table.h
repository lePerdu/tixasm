/**
 * @file reloc_table.h
 * @author Zach Peltzer
 * @date Created: Mon, 05 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#ifndef RELOC_TABLE_H_
#define RELOC_TABLE_H_

#include "expr.h"
#include "section.h"
#include "symbol_table.h"
#include "vector.h"

enum reloc_type {
    RT_UNDEF = 0,

    /**
     * For jr and djnz.
     * The relocation entry's value is set to the address directly after the
     * instruction. This is subtracted from the symbol's location to get the
     * final value; the result must be representable as a signed byte.
     */
    RT_REL_JUMP = 1,

    /* 8 and 16-bit integers. U means unsigned, S means signed, neither means
     * the value can be interpreted as either signed or unnsigned (for an 8-bit
     * integer, this means the value could range from -128 to +255).
     */
    RT_8_BIT = 2,
    RT_U_8_BIT = 3,
    RT_S_8_BIT = 4,
    RT_16_BIT = 5,
    RT_U_16_BIT = 6,
    RT_S_16_BIT = 7,

    RT_RST,
    RT_IM,

    /**
     * For use only in assembling.
     * Indicates that the relocation points to an expression, not a symbol.
     */
    RT_EXPR = 0x80,
};

struct reloc_ent {
    enum reloc_type type;
    enum section sec;
    int offset;

    /**
     * Type-dependent value used to calculate relocation values.
     * For symbols, this is an addend to the symbol.
     * For RT_REL_JUMP, this is the program counter which the relative jump is
     * relative to.
     * For RT_RST and RT_IM, this is the instruction byte without the value
     * applied.
     */
    int value;

    union {
        const struct symbol_ent *sym;
        struct expr_node *expr;
    };
};

struct reloc_table {
    struct vector relocs;
};


/**
 * Determines whether a value is in the allowed range for a specified relocation
 * type.
 * @param type Relocation type.
 * @param value Value to check.
 * @return true if the value is in range, false otherwise.
 */
int reltab_in_range(enum reloc_type type, int value);

/**
 * Initialize a relocation table.
 * @param rt Table to initialize.
 * @return 0 on success, -1 on failure.
 */
int reltab_init(struct reloc_table *rt);

/**
 * Destroys (frees) a relocation table and all of its entries.
 * @param rt Table to destroy.
 */
void reltab_destroy(struct reloc_table *rt);

/**
 * Gets the number of elements in a relocation table.
 * @param rt Table to get the size of.
 * @return Number of elements in @p rt.
 */
size_t reltab_get_size(const struct reloc_table *rt);

/**
 * Gets the relocation entry at an index in a table.
 * @param rt Relocation table to look in.
 * @param idx Index of the entry.
 * @param The entry at @p idx, or NULL if the index is out of range.
 */
const struct reloc_ent *reltab_get(const struct reloc_table *rt, int idx);

/**
 * Removes a relocation entry at an index.
 * This should only be done once the relocation was retrieved and resolved.
 * @param rt Relocation table to remove from.
 * @param idx Index to remove.
 * @return 0 if the entry was removed, -1 if the index is out of range.
 */
int reltab_remove(struct reloc_table *rt, int idx);

/**
 * Adds an entry to a relocation table referencing a symbol.
 * @param rt Relocation table to add to.
 * @param type Type of the relocation.
 * @param sec Section of the relocation.
 * @param offset Offset in the section of the data to relocate.
 * @param value Value used to calculate offsets/values depending on the
 * relocation type.
 * @param symbol Symbol the relocation references.
 */
int reltab_add_sym(struct reloc_table *rt,
        enum reloc_type type, enum section sec, int offset, int value,
        const struct symbol_ent *symbol);

/**
 * Adds an entry to a relocation table referencing an expression.
 * A deep clone of the expression will be made, so the original one can be freed
 * and/or modified.
 * @param rt Relocation table to add to.
 * @param type Type of the relocation.
 * @param sec Section of the relocation.
 * @param offset Offset in the section of the data to relocate.
 * @param value Value used to calculate offsets/values depending on the
 * relocation type.
 * @param expr Expression the relocation references.
 */
int reltab_add_expr(struct reloc_table *rt,
        enum reloc_type type, enum section sec, int offset, int value,
        const struct expr_node *expr);

#endif /* RELOC_TABLE_H_ */

/* vim: set tw=80 ft=c: */
