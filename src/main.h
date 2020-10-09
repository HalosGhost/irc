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

#include "irc.h"

static volatile sig_atomic_t exit_status = EXIT_SUCCESS;
static volatile sig_atomic_t caught_signum;

static char * server = "chat.freenode.net";
static char * port = "6667";
static char * pass = NULL;
static char * nick = "guest9899";
static char * ident = "guest9899";
static char * gecos = "a new client";

static char * logpath = "./log";

static char * channels [] = {
    "##hgtest"
};

signed
handle_server_message (FILE *, signed, char *);

void
signal_handler (signed);

#endif
