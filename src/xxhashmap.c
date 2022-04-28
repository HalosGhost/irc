#include "xxhashmap.h"

void
xxhashmap_init (xxhashmap * map, size_t magnitude) {

    map->capacity = magnitude;
    map->buckets = calloc(1 << magnitude, sizeof(struct linked_list *));
}

bool
xxhashmap_contains (xxhashmap * map, char * key) {

    unsigned idx = hashindex(map->capacity, key);

    struct linked_list * node = map->buckets[idx];
    while ( node ) {
        if ( !strcmp(node->key, key) ) {
            return true;
        }
        node = node->next;
    }

    return false;
}

WINDOW *
xxhashmap_get (xxhashmap * map, char * key) {

    unsigned idx = hashindex(map->capacity, key);

    struct linked_list * node = map->buckets[idx];
    while ( node ) {
        if ( !strcmp(node->key, key) ) {
            return node->val;
        }
        node = node->next;
    }

    return NULL;
}

void
xxhashmap_insert (xxhashmap * map, char * key, WINDOW * val) {

    unsigned idx = hashindex(map->capacity, key);

    if ( !map->buckets[idx] ) {
        ll_init(map->buckets[idx]);

        size_t keylen = strlen(key) + 1;
        map->buckets[idx]->key = calloc(keylen, sizeof(char));
        memcpy(map->buckets[idx]->key, key, keylen);

        map->buckets[idx]->val = val;
        return;
    }

    struct linked_list * node = map->buckets[idx];
    struct linked_list * last;
    while ( node ) {
        if ( !strcmp(node->key, key) ) {
            break;
        }
        last = node;
        node = node->next;
    }

    // no match
    if ( !node ) {
        ll_append(last, key, val);
        return;
    }

    node->val = val;
}

void
xxhashmap_delete (xxhashmap * map, char * key) {

    unsigned idx = hashindex(map->capacity, key);

    struct linked_list * node = map->buckets[idx];
    struct linked_list * last;
    while ( node ) {
        if ( !strcmp(node->key, key) ) {
            break;
        }
        last = node;
        node = node->next;
    }

    if ( node ) {
        last->next = node->next;
        free(node->key);
        free(node);
    }
}

void
xxhashmap_free (xxhashmap * map) {

    if ( !map->buckets ) {
        return;
    };

    for ( size_t i = 0; i < lower_n_mask(map->capacity); ++i ) {
        if ( !map->buckets[i] ) { continue; }
        ll_free(map->buckets[i]);
    }

    map->capacity = 0;

    if ( map->buckets ) { free(map->buckets); }
    map->buckets = 0;
}

