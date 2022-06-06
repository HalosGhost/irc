#include "list.h"

void
ll_append (struct linked_list * ll, char * key, FILE * log) {

    struct linked_list * node;
    ll_init(node);

    size_t keylen = strlen(key) + 1;
    node->name = calloc(1, keylen);
    memcpy(node->name, key, keylen);

    node->buf.log = log;
    ring_init(&(node->buf.hist));

    node->next = NULL;
    ll->next = node;
}

void
ll_free (struct linked_list * ll) {

    if ( ll->next ) {
        ll_free(ll->next);
    }

    free(ll->buf.hist);
    free(ll->name);
}
