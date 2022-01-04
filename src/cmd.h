#if !defined(CMD_H)
#define CMD_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

#define FOR_EACH_BUILTIN \
    X(C_HELP, help) \
    X(C_CONNECT, connect) \
    X(C_JOIN, join) \
    X(C_DISCONNECT, disconnect) \
    X(C_QUIT, quit) \
    X(C_MESSAGE, message) \
    X(C_UNKNOWN, unknown)

#define X(c, p) c,
enum cmd_builtin {
    FOR_EACH_BUILTIN
};
#undef X

#define X(c, p) [c] = #p,
static const char * builtin_name [] = {
    FOR_EACH_BUILTIN
};
#undef X

enum cmd_builtin
identify_cmd (const char *);

#endif
