/*
  +----------------------------------------------------------------------+
  | ext-tui: String interning pool implementation                        |
  +----------------------------------------------------------------------+
  | Uses FNV-1a hash for fast, good distribution.                        |
  | Collision resolution via chaining.                                   |
  +----------------------------------------------------------------------+
*/

#include "intern.h"
#include <stdlib.h>
#include <string.h>

/**
 * FNV-1a hash function.
 * Fast with good distribution for short strings.
 */
static uint32_t fnv1a_hash(const char *str, size_t len)
{
    uint32_t hash = 2166136261u;  /* FNV offset basis */
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= 16777619u;        /* FNV prime */
    }
    return hash;
}

/**
 * Find entry in bucket chain.
 */
static tui_intern_entry* find_entry(tui_intern_entry *entry, const char *str,
                                    size_t len, uint32_t hash)
{
    while (entry) {
        if (entry->hash == hash && entry->len == len &&
            memcmp(entry->str, str, len) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    return NULL;
}

int tui_intern_pool_init(tui_intern_pool *pool)
{
    if (!pool) return -1;

    memset(pool, 0, sizeof(tui_intern_pool));
    return 0;
}

void tui_intern_pool_shutdown(tui_intern_pool *pool)
{
    if (!pool) return;

    /* Free all entries in all buckets */
    for (int i = 0; i < INTERN_POOL_BUCKETS; i++) {
        tui_intern_entry *entry = pool->buckets[i];
        while (entry) {
            tui_intern_entry *next = entry->next;
            free(entry->str);
            free(entry);
            entry = next;
        }
        pool->buckets[i] = NULL;
    }

    pool->total_strings = 0;
    pool->total_bytes = 0;
}

void tui_intern_pool_reset(tui_intern_pool *pool)
{
    /* For now, reset is the same as shutdown.
     * In the future we could keep the entries but reset refcounts. */
    tui_intern_pool_shutdown(pool);
    pool->intern_hits = 0;
    pool->intern_misses = 0;
}

const char* tui_intern(tui_intern_pool *pool, const char *str, size_t len)
{
    if (!pool || !str || len == 0 || len > INTERN_MAX_STRING_LEN) {
        return NULL;
    }

    uint32_t hash = fnv1a_hash(str, len);
    int bucket = (int)(hash % INTERN_POOL_BUCKETS);

    /* Search for existing entry */
    tui_intern_entry *entry = find_entry(pool->buckets[bucket], str, len, hash);
    if (entry) {
        entry->refcount++;
        pool->intern_hits++;
        return entry->str;
    }

    /* Create new entry */
    entry = malloc(sizeof(tui_intern_entry));
    if (!entry) return NULL;

    entry->str = malloc(len + 1);
    if (!entry->str) {
        free(entry);
        return NULL;
    }

    memcpy(entry->str, str, len);
    entry->str[len] = '\0';
    entry->len = len;
    entry->hash = hash;
    entry->refcount = 1;

    /* Insert at head of bucket chain */
    entry->next = pool->buckets[bucket];
    pool->buckets[bucket] = entry;

    pool->total_strings++;
    pool->total_bytes += (int64_t)(len + 1);
    pool->intern_misses++;

    return entry->str;
}

const char* tui_intern_str(tui_intern_pool *pool, const char *str)
{
    if (!str) return NULL;
    return tui_intern(pool, str, strlen(str));
}

void tui_intern_release(tui_intern_pool *pool, const char *str)
{
    if (!pool || !str) return;

    /* We need to find the entry that contains this string.
     * Since we store heap-allocated strings, we search by pointer. */
    for (int i = 0; i < INTERN_POOL_BUCKETS; i++) {
        tui_intern_entry *prev = NULL;
        tui_intern_entry *entry = pool->buckets[i];

        while (entry) {
            if (entry->str == str) {
                entry->refcount--;
                if (entry->refcount <= 0) {
                    /* Remove from chain */
                    if (prev) {
                        prev->next = entry->next;
                    } else {
                        pool->buckets[i] = entry->next;
                    }

                    pool->total_strings--;
                    pool->total_bytes -= (int64_t)(entry->len + 1);

                    free(entry->str);
                    free(entry);
                }
                return;
            }
            prev = entry;
            entry = entry->next;
        }
    }
    /* String not found - silently ignore (might have been freed already) */
}

int tui_intern_is_interned(tui_intern_pool *pool, const char *str)
{
    if (!pool || !str) return 0;

    for (int i = 0; i < INTERN_POOL_BUCKETS; i++) {
        tui_intern_entry *entry = pool->buckets[i];
        while (entry) {
            if (entry->str == str) {
                return 1;
            }
            entry = entry->next;
        }
    }
    return 0;
}
