/**
 * @file hash_table.c
 * @author Zach Peltzer
 * @date Created: Sun, 04 Feb 2018
 * @date Last Modified: Mon, 05 Feb 2018
 */

#include <stdlib.h>
#include <string.h>

#include "hash_table.h"

/**
 * Hash function for strings.
 * For a string of characters [c0, c1, c2, ..., c{n-1}], this computes:
 *  h = c0*31^{n-1} + c1*31^{n-2} + ... + c{n-1}
 *
 * @param key String to hash.
 * @return Hash code for @p key.
 */
static int hash_str(const char *key) {
    int hash = 0;
    for (int i = 0; key[i]; i++) {
        hash = 31 * hash + key[i];
    }

    return hash;
}

/**
 * Allocates a new bucket, and a copy of the key.
 */
static struct hash_bucket *hash_bucket_alloc(const char *key, void *data) {
    struct hash_bucket *bucket = malloc(sizeof(*bucket));
    if (!bucket) {
        return NULL;
    }

    bucket->key = strdup(key);
    if (!bucket->key) {
        free(bucket);
        return NULL;
    }

    bucket->data = data;
    bucket->next = NULL;
    return bucket;
}

/**
 * Frees a bucket and its key.
 */
static void hash_bucket_free(struct hash_bucket *bucket) {
    if (!bucket) {
        return;
    }

    free(bucket->key);
    free(bucket);
}

int hashtab_init_size(struct hash_table *ht, size_t bucket_count) {
    if (!ht) {
        return -1;
    }

    ht->buckets = calloc(bucket_count, sizeof(ht->buckets[0]));
    if (!ht->buckets) {
        return -1;
    }

    ht->bucket_count = bucket_count;
    ht->size = 0;
}

void hashtab_destroy(struct hash_table *ht) {
    if (!ht) {
        return;
    }

    hashtab_clear(ht);

    free(ht->buckets);
    return;
}

int hashtab_has(const struct hash_table *ht, const char *key) {
    if (!ht || !key) {
        return 0;
    }

    int idx = hash_str(key) % ht->bucket_count;
    struct hash_bucket *bucket = ht->buckets[idx];
    while (bucket) {
        if (strcmp(key, bucket->key) == 0) {
            return 1;
        }

        bucket = bucket->next;
    }

    return 0;
}

void *hashtab_get(const struct hash_table *ht, const char *key) {
    if (!ht || !key) {
        return NULL;
    }

    int idx = hash_str(key) % ht->bucket_count;
    struct hash_bucket *bucket = ht->buckets[idx];
    while (bucket) {
        if (strcmp(key, bucket->key) == 0) {
            return bucket->data;
        }

        bucket = bucket->next;
    }

    return NULL;
}

int hashtab_set(struct hash_table *ht, const char *key, void *data) {
    if (!ht || !key) {
        return -1;
    }

    int idx = hash_str(key) % ht->bucket_count;
    struct hash_bucket *bucket = ht->buckets[idx];
    if (!bucket) {
        /* First node in the index */
        ht->buckets[idx] = hash_bucket_alloc(key, data);
        ht->size++;

        /* Return whether it was successful */
        return ht->buckets[idx] != NULL;
    }

    while (bucket) {
        if (strcmp(key, bucket->key) == 0) {
            bucket->data = data;
            return 0;
        }

        /* If at the end of the list and no matches have been found, add a new
         * node.
         */
        if (!bucket->next) {
            bucket->next = hash_bucket_alloc(key, data);
            ht->size++;

            /* Return whether it was successful */
            return bucket->next != NULL;
        }

        bucket = bucket->next;
    }

    return -1;
}

int hashtab_remove(struct hash_table *ht, const char *key) {
    if (!ht || !key) {
        return -1;
    }

    int idx = hash_str(key) % ht->bucket_count;
    struct hash_bucket *bucket = ht->buckets[idx];
    if (!bucket) {
        return -1;
    }

    struct hash_bucket *prev = NULL;
    do {
        if (strcmp(key, bucket->key) == 0) {
            /* If at the head of the list, set directly to the bucket list */
            if (prev) {
                prev->next = bucket->next;
            } else {
                ht->buckets[idx] = bucket->next;
            }

            hash_bucket_free(bucket);
            return 0;
        }

        prev = bucket;
        bucket = bucket->next;
    } while (bucket);

    return -1;
}

void hashtab_clear(struct hash_table *ht) {
    if (!ht) {
        return;
    }

    for (int i = 0; i < ht->bucket_count; i++) {
        struct hash_bucket *bucket = ht->buckets[i];
        while (bucket) {
            /* It is bad practice to access the next field of bucket after it is
             * free()d, so get it here
             */
            struct hash_bucket *next = bucket->next;
            hash_bucket_free(bucket);
            bucket = next;
        }

        ht->buckets[i] = NULL;
    }

    ht->size = 0;
}

void hashtab_free_all(struct hash_table *ht) {
    if (!ht) {
        return;
    }

    for (int i = 0; i < ht->bucket_count; i++) {
        struct hash_bucket *bucket = ht->buckets[i];
        while (bucket) {
            /* It is bad practice to access the next field of bucket after it is
             * free()d, so get it here
             */
            struct hash_bucket *next = bucket->next;
            free(bucket->data);
            hash_bucket_free(bucket);
            bucket = next;
        }

        ht->buckets[i] = NULL;
    }

    ht->size = 0;
}

/* vim: set tw=80 ft=c: */
