/**
 * @file vector.h
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Mon, 05 Feb 2018
 */

#ifndef VECTOR_H_
#define VECTOR_H_

#include <stdlib.h>

/**
 * Default initial capacity of a vector.
 */
#define VECTOR_DEF_INIT_CAP 4

/**
 * Dynamically sized array-like container.
 */
struct vector {
    /**
     * Number of elements in the vector.
     */
    size_t size;

    /**
     * Current capacity of the vector.
     */
    size_t capacity;

    /**
     * Elements of the vector.
     */
    void **elements;
};

/**
 * Initializes a vector with a default capacity.
 * @param v Vector to initialize.
 * @return 0 on success, -1 on failure.
 */
int vector_init(struct vector *v);

/**
 * Initializes a vector with a specified capacity.
 * @param v Vector to initialize.
 * @param capacity Initial capacity of the vector.
 * @return 0 on success, -1 on failure.
 */
int vector_init_cap(struct vector *v, size_t capacity);

/**
 * Destroys (frees) a vector.
 * This does NOT call free() on the vector's elements. Use vector_free_all() for
 * that.
 * @param v Vector to destroy.
 */
void vector_destroy(struct vector *v);

/**
 * Gets an element from a vector.
 * @param v Vector to get from.
 * @param idx Index of the element to get.
 * @return The element at @p idx, or NULL if the index is out of bounds.
 */
void *vector_get(const struct vector *v, int idx);

/**
 * Sets an element of a vector.
 * This grows the vector if the index is out of range.
 * @param v Vector to set in.
 * @param idx Index to set at.
 * @param data Element to set.
 * @return 0 on success, -1 on failure.
 */
int vector_set(struct vector *v, int idx, void *data);

/**
 * Adds an element to the end of a vector.
 * @param v Vector to set in.
 * @param data Element to set.
 * @return 0 on success, -1 on failure.
 */
int vector_add(struct vector *v, void *data);

/**
 * Removes an element from a vector.
 * @param v Vector to remove from.
 * @param idx Index of the element to remove.
 * @return 0 on success, -1 if the index is out of bounds.
 */
int vector_remove(struct vector *v, int idx);

/**
 * Removes all elements from a vector.
 * This does NOT call free() on the vector's elements. Use vector_free_all() for
 * that.
 * @param v Vector to clear.
 */
void vector_clear(struct vector *v);

/**
 * Removes and calls free() on all elements from a vector.
 * This does not deallocate (all) memory of the vector. Use vector_destroy() for
 * that.
 * @param v Vector to clear.
 */
void vector_free_all(struct vector *v);

#endif /* VECTOR_H_ */

/* vim: set tw=80 ft=c: */
