/**
 * @file symbol_table.c
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"

struct symbol_table *symbol_table;

static struct symbol_ent *syment_alloc(const char *name,
        enum section sec, enum symbol_type type, int value) {
    struct symbol_ent *ent = malloc(sizeof(*ent));
    if (!ent) {
        return NULL;
    }

    ent->name = strdup(name);
    ent->sec = sec;
    ent->type = type;
    ent->value = value;
    return ent;
}

static void syment_free(struct symbol_ent *ent) {
    if (!ent) {
        return;
    }

    free(ent->name);
    free(ent);
}

int symtab_init(struct symbol_table *st) {
    if (!st) {
        return -1;
    }

    return hashtab_init_size(&st->symbols, 128);
}

void symtab_destroy(struct symbol_table *st) {
    if (!st) {
        return;
    }

    hashtab_free_all(&st->symbols);
    hashtab_destroy(&st->symbols);
}

const struct symbol_ent *symtab_search(
        const struct symbol_table *st, const char *name) {
    if (!st) {
        return NULL;
    }

    return hashtab_get(&st->symbols, name);
}

const struct symbol_ent *symtab_search_len(const struct symbol_table *st,
        const char *name, int name_len) {
    /* Just copy the string then free it later */
    char *name_nt = strndup(name, name_len);
    const struct symbol_ent *ret = symtab_search(st, name_nt);
    free(name_nt);
    return ret;
}

const struct symbol_ent *symtab_add(struct symbol_table *st, const char *name,
        enum symbol_type type, enum section sec, int value) {
    /* I know it's REALLY bad to cast away const-ness, but it's okay here since
     * this structure "owns" the entry anyway: symtab_search() could be replaced
     * with hashtab_get() with the same effect.
     */
    struct symbol_ent *ent = (struct symbol_ent *) symtab_search(st, name);

    if (!ent) {
        ent = syment_alloc(name, sec, type, value);
        if (!ent) {
            return NULL;
        }

        if (hashtab_set(&st->symbols, name, ent) < 0) {
            syment_free(ent);
            return NULL;
        }

        /* Sucessfully allocated and added */
        return ent;
    } else if (ent->type == ST_UNDEF) {
        /* This will overwrite an undefined symbol */
        ent->sec = sec;
        ent->type = type;
        ent->value = value;
        return ent;
    } else {
        /* Symbol exists */
        return NULL;
    }
}

const struct symbol_ent *symtab_add_len(struct symbol_table *st,
        const char *name, int name_len,
        enum symbol_type type, enum section sec, int value) {
    /* Just copy the string then free it later */
    char *name_nt = strndup(name, name_len);
    const struct symbol_ent *ret =
        symtab_add(st, name_nt, sec, type, value);
    free(name_nt);
    return ret;
}

/* vim: set tw=80 ft=c: */
