#if !defined(XXHASHMAP_H)
#define XXHASHMAP_H

#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#include <xxhash.h>

#include "list.h"

typedef struct xxhashmap {
    size_t capacity;
    struct linked_list ** buckets;
} xxhashmap;

#define lower_n_mask(x) ((1 << (x)) - 1)
#define hashindex(sz, k) \
    (XXH3_64bits((k), strlen(k)) & lower_n_mask(sz))

void
xxhashmap_init (xxhashmap *, size_t);

void
xxhashmap_insert (xxhashmap *, char *, FILE *);

bool
xxhashmap_contains (xxhashmap *, char *);

struct linked_list *
xxhashmap_get (xxhashmap *, char *);

void
xxhashmap_delete (xxhashmap *, char *);

void
xxhashmap_free (xxhashmap *);

#endif
