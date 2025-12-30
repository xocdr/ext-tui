/*
  +----------------------------------------------------------------------+
  | ext-tui: Hash-based key map for reconciler                          |
  | O(1) average lookup vs O(n) linear scan                             |
  +----------------------------------------------------------------------+
*/

#ifndef TUI_KEYMAP_H
#define TUI_KEYMAP_H

#include "node.h"
#include <stdint.h>

/* Initial hash table size (must be power of 2) */
#define TUI_KEYMAP_INITIAL_SIZE 16

/* Load factor threshold (expand at 75% full) */
#define TUI_KEYMAP_LOAD_FACTOR 0.75

/* Entry in the hash map */
typedef struct {
    char *key;              /* Key string (not owned, just referenced) */
    tui_node *node;         /* Associated node */
    int old_index;          /* Original index in old children array */
    int matched;            /* 1 if this entry has been matched */
    uint32_t hash;          /* Cached hash value */
} tui_keymap_entry;

/* Hash-based key map */
typedef struct {
    tui_keymap_entry *buckets;  /* Hash table buckets */
    int capacity;               /* Number of buckets (power of 2) */
    int count;                  /* Number of entries */
    int from_pool;              /* 1 if buckets from pool */
} tui_keymap;

/**
 * DJB2 hash function - fast and good distribution for strings.
 */
static inline uint32_t tui_key_hash(const char *str)
{
    uint32_t hash = 5381;
    int c;
    while ((c = (unsigned char)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

/**
 * Create a new key map with given initial capacity.
 * Capacity is rounded up to next power of 2.
 * Returns NULL on allocation failure.
 */
tui_keymap* tui_keymap_create(int hint_capacity);

/**
 * Destroy a key map and free its memory.
 * Releases pooled memory if applicable.
 */
void tui_keymap_destroy(tui_keymap *map);

/**
 * Insert a key-node pair into the map.
 * Returns 1 on success, 0 on failure.
 * Note: Does not copy key string, just stores pointer.
 */
int tui_keymap_insert(tui_keymap *map, const char *key, tui_node *node, int old_index);

/**
 * Find an entry by key.
 * Returns pointer to entry, or NULL if not found.
 * Entry can be modified (e.g., set matched flag).
 */
tui_keymap_entry* tui_keymap_find(tui_keymap *map, const char *key);

/**
 * Mark an entry as matched.
 */
static inline void tui_keymap_mark_matched(tui_keymap_entry *entry)
{
    if (entry) entry->matched = 1;
}

#endif /* TUI_KEYMAP_H */
