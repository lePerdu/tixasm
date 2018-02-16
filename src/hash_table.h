/**
 * @file hash_table.h
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Mon, 05 Feb 2018
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <stdlib.h>

/**
 * Default number of buckets for a hash_table.
 * A larger number of buckets uses more space, but operations will be more
 * efficient.
 */
#define HASHTAB_DEF_BUCKET_COUNT 32

/**
 * Bucket to store each element in a hash_table.
 * This is basically a linked list node, storing the exact key of the entry in
 * addition to the element.
 */
struct hash_bucket {
    char *key;
    void *data;
    struct hash_bucket *next;
};

/**
 * Hash table using buckets with linked lists.
 * Values are stored as void pointers, and so can be used for any data type,
 * though keys are restricted to being strings, as they must be copied to avoid
 * dangling pointers.
 * TODO Adjust the number of buckets when the table fills up to a certain point.
 */
struct hash_table {

    /**
     * Size (number of elements) of the table.
     */
    size_t size;

    /**
     * Number of buckets in the table.
     */
    size_t bucket_count;

    /**
     * Buckets in the table. Each points to the head of a linked list.
     */
    struct hash_bucket **buckets;
};

/**
 * Initializes a hash table with the default number of buckets.
 * @param ht Table to initialize.
 * @return 0 on success, -1 on failure.
 */
inline int hashtab_init(struct hash_table *ht);

/**
 * Initializes a hash table with a specified number of buckets.
 * @param ht Table to initialize.
 * @param bucket_count Number of buckets to use.
 * @return 0 on success, -1 on failure.
 */
int hashtab_init_size(struct hash_table *ht, size_t bucket_count);

/**
 * Destroys a hash table, freeing its memeory.
 * This does NOT call free() on any of its elements. To do so, use
 * hashtab_free_all().
 */
void hashtab_destroy(struct hash_table *ht);

/**
 * Determines whether or not a key is set in a hash table.
 * @param ht Table to test.
 * @param key Key to test for.
 * @return true (1) if the key is found, false (0) if not.
 */
int hastab_has(const struct hash_table *ht, const char *key);

/**
 * Gets an element from a hash table.
 * @param ht Table to get from.
 * @param key Key of the element to get.
 * @return The element for @p key, or NULL if no such element exists.
 */
void *hashtab_get(const struct hash_table *ht, const char *key);

/**
 * Sets an element in a hash table.
 * @param ht Table to set in.
 * @param key Key of the element to set.
 * @param data Element to set.
 * @return 0 on success, -1 on failure.
 */
int hashtab_set(struct hash_table *ht, const char *key, void *data);

/**
 * Removes an element from a hash table.
 * TODO Return the removed element? For the purposes of tixasm, elements should
 * never be removed, so this doesn't really matter much.
 * @param ht Table to remove from.
 * @param key Key of the element to remove.
 * @return 0 on success, -1 if the key is not in the table.
 */
int hashtab_remove(struct hash_table *ht, const char *key);

/**
 * Removes all elements from a hash table.
 * This does NOT call free() on any of its elements. To do so, use
 * hashtab_free_all().
 * @param ht Table to clear.
 */
void hashtab_clear(struct hash_table *ht);

/**
 * Removes and calls free() on all elements in a hash table.
 * @param ht Table to clear/free.
 */
void hashtab_free_all(struct hash_table *ht);

inline int hashtab_init(struct hash_table *ht) {
    return hashtab_init_size(ht, HASHTAB_DEF_BUCKET_COUNT);
}

#endif /* HASH_TABLE_H_ */

/* vim: set tw=80 ft=c: */
