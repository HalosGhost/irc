#if !defined(CMD_H)
#define CMD_H

#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "irc.h"

// todo: fill out logging format specifiers
#define FOR_EACH_BUILTIN \
    X(C_HELP, help, IRC_UNKNOWN, NULL, NULL) \
    X(C_CONNECT, connect, IRC_UNKNOWN, NULL, NULL) \
    X(C_JOIN, join, JOIN, NULL, NULL) \
    X(C_DISCONNECT, disconnect, IRC_UNKNOWN, NULL, NULL) \
    X(C_QUIT, quit, IRC_UNKNOWN, NULL, NULL) \
    X(C_MESSAGE, message, PRIVMSG, "<%s> %s\n", NULL) \
    X(C_ACTION, me, PRIVMSG, "%s %s\n", NULL) \
    X(C_BUFFERS, buffers, IRC_UNKNOWN, NULL, NULL) \
    X(C_GOTO, goto, IRC_UNKNOWN, NULL, NULL) \
    X(C_QUERY, query, PRIVMSG, NULL, NULL) \
    X(C_TOPIC, topic, TOPIC, NULL, NULL) \
    X(C_NAMES, names, NAMES, NULL, NULL) \
    X(C_UNKNOWN, unknown, IRC_UNKNOWN, "%s", NULL)

#define X(_c, _p, _i, _b, _l) _c,
enum cmd_builtin {
    FOR_EACH_BUILTIN
};
#undef X

#define X(_c, _p, _i, _b, _l) [_c] = paste(_p),
static const char * builtin_name [] = {
    FOR_EACH_BUILTIN
};
#undef X

#define X(_c, _p, _i, _b, _l) [_c] = _i,
static const enum irc_command builtin_irc [] = {
    FOR_EACH_BUILTIN
};
#undef X

#define str(_x) paste(_x)

#define X(_c, _p, _i, _b, _l) [_c] = sizeof str(_p),
static const size_t builtin_offset [] = {
    FOR_EACH_BUILTIN
};
#undef X

#define X(_c, _p, _i, _b, _l) [_c] = _b,
static const char * builtin_buf_fmt [] = {
    FOR_EACH_BUILTIN
};
#undef X

#define X(_c, _p, _i, _b, _l) [_c] = _l,
static const char * builtin_log_fmt [] = {
    FOR_EACH_BUILTIN
};
#undef X

enum cmd_builtin
identify_cmd (const char *);

// todo: extract command case bodies from main here

#endif
