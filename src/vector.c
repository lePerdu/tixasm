/**
 * @file vector.c
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Tue, 06 Feb 2018
 */

#include <stdlib.h>
#include <string.h>

#include "vector.h"

/**
 * Truncates a vector to a new capacity.
 * If the new capacity is less than the current size, indices after the new
 * capacity will be lost.
 * TODO Export this function?
 * @param v Vector to truncate.
 * @param capacity New capacity to set.
 * @return 0 on success, -1 on failure.
 */
static int vector_trunc(struct vector *v, size_t capacity);

int vector_init(struct vector *v) {
    return vector_init_cap(v, VECTOR_DEF_INIT_CAP);
}

int vector_init_cap(struct vector *v, size_t capacity) {
    if (!v) {
        return -1;
    }

    v->size = 0;
    v->elements = NULL;
    return vector_trunc(v, capacity);
}

void vector_destroy(struct vector *v) {
    if (!v) {
        return;
    }

    free(v->elements);
}

void *vector_get(const struct vector *v, int idx) {
    if (!v || idx < 0 || idx >= v->size) {
        return NULL;
    }

    return v->elements[idx];
}

int vector_set(struct vector *v, int idx, void *data) {
    if (!v || idx < 0) {
        return -1;
    }

    if (idx >= v->size) {
        if (idx >= v->capacity) {
            /* Multiply the capacity by 2 until it fits idx */
            int capacity = v->capacity * 2;
            while (capacity < idx) {
                capacity *= 2;
            }

            if (vector_trunc(v, capacity) < 0) {
                return -1;
            }
        }

        /* Fill unset elements with NULL */
        memset(&v->elements[v->size], 0,
                sizeof(v->elements[0]) * (idx - v->size));
        v->size = idx + 1;
    }

    v->elements[idx] = data;
    return 0;
}

int vector_add(struct vector *v, void *data) {
    if (!v) {
        return -1;
    }

    return vector_set(v, v->size, data);
}

int vector_remove(struct vector *v, int idx) {
    if (!v || idx < 0 || idx >= v->size) {
        return -1;
    }

    memmove(&v->elements[idx], &v->elements[idx+1], v->size - (idx+1));
    v->size--;
    if (v->size * 2 < v->capacity) {
        vector_trunc(v, v->capacity / 2);
    }

    return 0;
}

void vector_clear(struct vector *v) {
    if (!v) {
        return;
    }

    v->size = 0;
}

void vector_free_all(struct vector *v) {
    if (!v) {
        return;
    }

    for (int i = 0; i < v->size; i++) {
        free(v->elements[i]);
    }

    v->size = 0;
}

int vector_trunc(struct vector *v, size_t capacity) {
    if (!v) {
        return -1;
    }

    /* It's pointless to allocate a really small array */
    if (capacity < VECTOR_DEF_INIT_CAP) {
        capacity = VECTOR_DEF_INIT_CAP;
    }

    /* Allocate first to make sure there is no error */
    void **elements = realloc(v->elements, sizeof(*elements) * capacity);
    if (!elements) {
        return -1;
    }

    v->elements = elements;
    v->capacity = capacity;
    if (v->size > capacity) {
        v->size = capacity;
    }

    return 0;
}

/* vim: set tw=80 ft=c: */
