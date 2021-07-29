#if !defined(MAIN_H)
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <ncurses.h>
#include <signal.h>
#include <stdbool.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>

#include "irc.h"
#include "cmd.h"

static volatile sig_atomic_t exit_status = EXIT_SUCCESS;
static volatile sig_atomic_t caught_signum;

static struct irc_server freenode = {
    .host = "chat.freenode.net",
    .port = "6667",
    .nick = "hgtest",
    .ident = "hgtest",
    .gecos = "🐼",
    .tls = true
};

static char * logpath = "./log";

static char * channels [] = {
    "##meskarune"
};

static char user_entry [IRC_MESSAGE_MAX + 1];

signed
handle_server_message (FILE *, BIO *, char *);

enum cmd_builtin
handle_local_message (FILE *, BIO *, char *);

void
signal_handler (signed);

#endif
