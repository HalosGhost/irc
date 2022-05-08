#if !defined(LIST_H)
#define LIST_H

#pragma once

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

struct linked_list {
    char * name;
    WINDOW * buf;
    FILE * log;
    struct linked_list * next;
};

#define ll_init(x) x = calloc(1, sizeof(struct linked_list))

void
ll_append (struct linked_list *, char *, WINDOW *, FILE *);

void
ll_free (struct linked_list *);

#endif