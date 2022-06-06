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
        if ( !strcmp(node->name, key) ) {
            return true;
        }
        node = node->next;
    }

    return false;
}

struct linked_list *
xxhashmap_get (xxhashmap * map, char * key) {

    unsigned idx = hashindex(map->capacity, key);

    struct linked_list * node = map->buckets[idx];
    while ( node ) {
        if ( !strcmp(node->name, key) ) {
            return node;
        }
        node = node->next;
    }

    return NULL;
}

void
xxhashmap_insert (xxhashmap * map, char * key, FILE * log) {

    unsigned idx = hashindex(map->capacity, key);

    if ( !map->buckets[idx] ) {
        ll_init(map->buckets[idx]);

        size_t keylen = strlen(key) + 1;
        map->buckets[idx]->name = calloc(keylen, sizeof(char));
        memcpy(map->buckets[idx]->name, key, keylen);

        map->buckets[idx]->buf.log = log;
        ring_init(&(map->buckets[idx]->buf.hist));
        return;
    }

    struct linked_list * node = map->buckets[idx];
    struct linked_list * last;
    while ( node ) {
        if ( !strcmp(node->name, key) ) {
            break;
        }
        last = node;
        node = node->next;
    }

    // no match
    if ( !node ) {
        ll_append(last, key, log);
        return;
    }

    node->buf.log = log;
    ring_init(&(node->buf.hist));
}

void
xxhashmap_delete (xxhashmap * map, char * key) {

    unsigned idx = hashindex(map->capacity, key);

    struct linked_list * node = map->buckets[idx];
    struct linked_list * last;
    while ( node ) {
        if ( !strcmp(node->name, key) ) {
            break;
        }
        last = node;
        node = node->next;
    }

    if ( node ) {
        last->next = node->next;
        free(node->name);
        free(node);
    }
}

void
xxhashmap_free (xxhashmap * map) {

    if ( !map->buckets ) {
        return;
    };

    for ( size_t i = 0; i < (size_t )lower_n_mask(map->capacity); ++i ) {
        if ( !map->buckets[i] ) { continue; }
        ll_free(map->buckets[i]);
    }

    map->capacity = 0;

    if ( map->buckets ) { free(map->buckets); }
    map->buckets = 0;
}

