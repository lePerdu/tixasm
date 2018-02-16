/**
 * @file symbol_table.h
 * @author Zach Peltzer
 * @date Created: Fri, 02 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#ifndef SYMTABLE_H_
#define SYMTABLE_H_

#include "section.h"
#include "hash_table.h"

enum symbol_type {
    ST_UNDEF = 0,

    /* These are types in ELF, and may be integrated in later */
    ST_FUNC,
    ST_OBJECT,
    ST_SECTION,

    ST_MACRO,
};

struct symbol_ent {
    char *name;
    enum symbol_type type;
    enum section sec;
    int value;
};

struct symbol_table {
    struct hash_table symbols;
};


/**
 * Initialize a symbol table.
 * @param st Table to initialize.
 * @return 0 on success, -1 on failure.
 */
int symtab_init(struct symbol_table *st);

/**
 * Destroys (frees) a symbol table and all of its entries.
 * @param st Table to destroy.
 */
void symtab_destroy(struct symbol_table *st);

/**
 * Searches for a symbol in the table.
 * @param st Symbol table to search in.
 * @param name Name of the symbol to search for.
 * @return A matching synbol entry, or NULL if none exist.
 */
const struct symbol_ent *symtab_search(
        const struct symbol_table *st, const char *name);

/**
 * Same as symtab_search(), but takes the length of the name.
 * @param st Symbol table to search in.
 * @param name Name of the symbol to search for.
 * @param len Length of the string at @p name.
 * @return A matching synbol entry, or NULL if none exist.
 */
const struct symbol_ent *symtab_search_len(const struct symbol_table *st,
        const char *name, int len);

/**
 * Adds an entry to the symbol table, if one has not already been defined for
 * the given name.
 * @param st Symbol table to add to.
 * @param name Name of the symbol to add. An entry of the same name may exist as
 * long as it is of type ST_UNDEF. In this case, the entry will be overwritten
 * with the new value (even if new type is also ST_UNDEF).
 * @param sec Section of the symbol.
 * @param type Type of the symbol.
 * @param value Value of the symbol.
 * @return The created symbol, or NULL if the symbol could not be created.
 */
const struct symbol_ent *symtab_add(struct symbol_table *st, const char *name,
        enum symbol_type type, enum section sec, int value);

/**
 * Adds an entry to the symbol table, if one has not already been defined for
 * the given name.
 * The name is passed with a length, instead of as null-terminated.
 * @param st Symbol table to add to.
 * @param name Name of the symbol to add. An entry of the same name may exist as
 * long as it is of type ST_UNDEF. In this case, the entry will be overwritten
 * with the new value (even if new type is also ST_UNDEF).
 * @param name_len Length of @p name.
 * @param sec Section of the symbol.
 * @param type Type of the symbol.
 * @param value Value of the symbol.
 * @return The created symbol, or NULL if the symbol could not be created.
 */
const struct symbol_ent *symtab_add_len(struct symbol_table *st,
        const char *name, int name_len,
        enum symbol_type type, enum section sec, int value);

#endif /* SYMTABLE_H_ */

/* vim: set tw=80 ft=c: */
