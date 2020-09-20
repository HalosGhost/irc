#if !defined(IRC_H)
#define IRC_H

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define IRC_LINE_ENDING "\r\n"
#define IRC_MESSAGE_MAX 512

#define FOR_EACH_COMMAND \
    X(INVITE, "%s %s") \
    X(JOIN, "%s") \
    X(KICK, "%s %s :%s") \
    X(MODE, "%s %s %s") \
    X(NAMES, "%s") \
    X(NICK, "%s") \
    X(NOTICE, "%s :%s") \
    X(PART, "%s") \
    X(PONG, "%s") \
    X(PRIVMSG, "%s :%s") \
    X(TOPIC, "%s %s") \
    X(USER, "%s 8 * :%s") \
    X(QUIT, "%s")

#define X(n, fmt) n,
enum irc_command {
    FOR_EACH_COMMAND
};
#undef X

#define X(n, fmt) [n] = #n,
static const char * irc_command_name[] = {
    FOR_EACH_COMMAND
};
#undef X

#define X(n, fmt) [n] = #n " " fmt,
static const char * irc_command_fmt[] = {
    FOR_EACH_COMMAND
};
#undef X

static char servername [IRC_MESSAGE_MAX + 1];

signed
irc_cmdf (enum irc_command, char *, va_list);

signed
irc_send (signed, enum irc_command, ...);

signed
irc_connect (char *, char *);

#endif

