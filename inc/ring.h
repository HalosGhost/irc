#if !defined(RING_H)
#define RING_H

#pragma once

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "irc.h"
#include "cmd.h"

struct ring {
    char ** rungs;
    size_t offset;
};

void
ring_init (struct ring **);

void
ring_insert (struct ring *, enum cmd_builtin, ...);

#endif
