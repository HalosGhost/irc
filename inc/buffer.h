#if !defined(BUFFER_H)
#define BUFFER_H

#pragma once

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#include "ring.h"

struct buffer {
    // todo: remove in favor of using hist
    WINDOW * win;
    FILE * log;
    struct ring * hist;
};

// todo: buffer_write to coordinate writing all the places

#endif
