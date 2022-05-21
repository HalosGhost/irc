#include "ring.h"

void
ring_init (struct ring ** ring) {

    struct ring * r = calloc(1, sizeof(struct ring));
    r->rungs = calloc(LINES - 2, IRC_MESSAGE_MAX);
    r->offset = 0;
    *ring = r;
}

void
ring_insert (struct ring * ring, enum cmd_builtin cmd, ...) {

    va_list args;
    va_start(args, cmd);

    const char * fmt = builtin_buf_fmt[cmd];
    if ( fmt ) {
        // todo: this will probably screw up on sigwinch
        vsnprintf(ring->rungs[ring->offset++ % LINES], IRC_MESSAGE_MAX, fmt, args);
    }
    va_end(args);
}
