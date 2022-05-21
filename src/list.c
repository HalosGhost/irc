#include "list.h"

void
ll_append (struct linked_list * ll, char * key, WINDOW * val, FILE * log) {

    struct linked_list * node;
    ll_init(node);

    size_t keylen = strlen(key) + 1;
    node->name = calloc(1, keylen);
    memcpy(node->name, key, keylen);

    node->buf.win = val;
    node->buf.log = log;

    node->next = NULL;
    ll->next = node;
}

void
ll_free (struct linked_list * ll) {

    if ( ll->next ) {
        ll_free(ll->next);
    }

    free(ll->name);
}
