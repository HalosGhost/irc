#if !defined(CMD_H)
#define CMD_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

// todo: fill out logging format specifiers
#define FOR_EACH_BUILTIN \
    X(C_HELP, help, NULL, NULL) \
    X(C_CONNECT, connect, NULL, NULL) \
    X(C_JOIN, join, NULL, NULL) \
    X(C_DISCONNECT, disconnect, NULL, NULL) \
    X(C_QUIT, quit, NULL, NULL) \
    X(C_MESSAGE, message, "<%s> %s\n", NULL) \
    X(C_ACTION, me, "%s %s\n", NULL) \
    X(C_BUFFERS, buffers, NULL, NULL) \
    X(C_GOTO, goto, NULL, NULL) \
    X(C_QUERY, query, NULL, NULL) \
    X(C_UNKNOWN, unknown, "%s", NULL)

#define X(c, p, b, l) c,
enum cmd_builtin {
    FOR_EACH_BUILTIN
};
#undef X

#define X(c, p, b, l) [c] = #p,
static const char * builtin_name [] = {
    FOR_EACH_BUILTIN
};
#undef X

#define X(c, p, b, l) [c] = b,
static const char * builtin_buf_fmt [] = {
    FOR_EACH_BUILTIN
};
#undef X

#define X(c, p, b, l) [c] = l,
static const char * builtin_log_fmt [] = {
    FOR_EACH_BUILTIN
};
#undef X

enum cmd_builtin
identify_cmd (const char *);

// todo: extract command case bodies from main here

#endif
