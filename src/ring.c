#include "ring.h"

void
ring_init (struct ring ** ring) {

    struct ring * r = calloc(1, sizeof(struct ring));
    r->rungs = calloc(LINES - 2, sizeof(char *));
    for ( size_t i = 0; i < LINES - 2; ++i ) {
        r->rungs[i] = calloc(IRC_MESSAGE_MAX, sizeof(char));
    }
    r->offset = LINES - 3; // ensures the first insert is at index 0
    *ring = r;
}

void
ring_insert (struct ring * ring, enum cmd_builtin cmd, ...) {

    va_list args;
    va_start(args, cmd);

    const char * fmt = builtin_buf_fmt[cmd];
    if ( fmt ) {
        // todo: this will probably screw up on sigwinch
        ring->offset = (ring->offset + 1) % (LINES - 2);
        vsnprintf(ring->rungs[ring->offset], IRC_MESSAGE_MAX, fmt, args);
    }
    va_end(args);
}

char *
ring_get (struct ring * ring, size_t index) {

    return ring->rungs[(index + ring->offset) % (LINES - 2)];
}
