/*
  +----------------------------------------------------------------------+
  | ext-tui: Hash-based key map implementation                          |
  | Uses open addressing with linear probing                            |
  +----------------------------------------------------------------------+
*/

#include "keymap.h"
#include "../pool/pool.h"
#include "php.h"
#include "php_tui.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Round up to next power of 2 */
static int next_power_of_2(int n)
{
    if (n <= 0) return TUI_KEYMAP_INITIAL_SIZE;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

tui_keymap* tui_keymap_create(int hint_capacity)
{
    tui_keymap *map = calloc(1, sizeof(tui_keymap));
    if (!map) return NULL;

    /* Round up to power of 2 for efficient modulo (use bitwise AND) */
    int capacity = next_power_of_2(hint_capacity > 0 ? hint_capacity : TUI_KEYMAP_INITIAL_SIZE);

    /* Account for load factor - allocate more buckets than entries */
    capacity = next_power_of_2((int)(capacity / TUI_KEYMAP_LOAD_FACTOR) + 1);

    map->capacity = capacity;
    map->count = 0;
    map->from_pool = 0;

    /* Try to get buckets from pool */
    if (TUI_G(pools)) {
        map->buckets = tui_key_map_pool_acquire(TUI_G(pools), capacity,
                                                  sizeof(tui_keymap_entry), &map->from_pool);
    } else {
        map->buckets = calloc(capacity, sizeof(tui_keymap_entry));
    }

    if (!map->buckets) {
        free(map);
        return NULL;
    }

    return map;
}

void tui_keymap_destroy(tui_keymap *map)
{
    if (!map) return;

    if (map->from_pool && TUI_G(pools)) {
        /* Return to pool */
        tui_key_map_pool_release(TUI_G(pools));
    } else {
        /* Free malloc'd memory */
        free(map->buckets);
    }
    free(map);
}

/* Resize the hash table when load factor exceeded */
static int tui_keymap_resize(tui_keymap *map)
{
    if (!map) return 0;

    /* Check for overflow */
    if (map->capacity > INT_MAX / 2) return 0;

    int new_capacity = map->capacity * 2;
    tui_keymap_entry *new_buckets = calloc(new_capacity, sizeof(tui_keymap_entry));
    if (!new_buckets) return 0;

    /* Rehash all existing entries */
    for (int i = 0; i < map->capacity; i++) {
        tui_keymap_entry *old_entry = &map->buckets[i];
        if (old_entry->key) {
            /* Find new position using cached hash */
            int idx = old_entry->hash & (new_capacity - 1);
            while (new_buckets[idx].key) {
                idx = (idx + 1) & (new_capacity - 1);
            }
            new_buckets[idx] = *old_entry;
        }
    }

    /* Free old buckets */
    if (map->from_pool && TUI_G(pools)) {
        tui_key_map_pool_release(TUI_G(pools));
    } else {
        free(map->buckets);
    }

    map->buckets = new_buckets;
    map->capacity = new_capacity;
    map->from_pool = 0;  /* Now using malloc'd memory */

    return 1;
}

int tui_keymap_insert(tui_keymap *map, const char *key, tui_node *node, int old_index)
{
    if (!map || !key) return 0;

    /* Check load factor and resize if needed */
    if (map->count >= (int)(map->capacity * TUI_KEYMAP_LOAD_FACTOR)) {
        if (!tui_keymap_resize(map)) return 0;
    }

    /* Compute hash */
    uint32_t hash = tui_key_hash(key);
    int idx = hash & (map->capacity - 1);

    /* Linear probing to find empty slot */
    while (map->buckets[idx].key) {
        /* Check if key already exists (shouldn't happen with unique keys) */
        if (map->buckets[idx].hash == hash &&
            strcmp(map->buckets[idx].key, key) == 0) {
            /* Update existing entry */
            map->buckets[idx].node = node;
            map->buckets[idx].old_index = old_index;
            map->buckets[idx].matched = 0;
            return 1;
        }
        idx = (idx + 1) & (map->capacity - 1);
    }

    /* Found empty slot */
    map->buckets[idx].key = (char *)key;  /* Don't copy, just reference */
    map->buckets[idx].node = node;
    map->buckets[idx].old_index = old_index;
    map->buckets[idx].matched = 0;
    map->buckets[idx].hash = hash;
    map->count++;

    return 1;
}

tui_keymap_entry* tui_keymap_find(tui_keymap *map, const char *key)
{
    if (!map || !key || map->count == 0) return NULL;

    uint32_t hash = tui_key_hash(key);
    int idx = hash & (map->capacity - 1);
    int start_idx = idx;

    /* Linear probing */
    do {
        tui_keymap_entry *entry = &map->buckets[idx];

        if (!entry->key) {
            /* Empty slot - key not found */
            return NULL;
        }

        if (entry->hash == hash && strcmp(entry->key, key) == 0) {
            /* Found it */
            return entry;
        }

        idx = (idx + 1) & (map->capacity - 1);
    } while (idx != start_idx);

    /* Wrapped around - shouldn't happen if load factor is maintained */
    return NULL;
}
