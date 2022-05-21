#if !defined(MAIN_H)
#define MAIN_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <locale.h>

#include "xxhashmap.h"
#include "irc.h"
#include "cmd.h"

static volatile sig_atomic_t exit_status = EXIT_SUCCESS;
static volatile sig_atomic_t caught_signum;

static char * server = "irc.libera.chat";
static char * port = "6667";
static char * pass = NULL;
static char * nick = "hg-x201";
static char * ident = "hg-x201";
static char * gecos = "a new client";

//static char * logpath = "./log";

static struct linked_list * chan;
static xxhashmap chanmap;
static xxhashmap * channels = &chanmap;

static char * autojoin [] = {
    "##meskarune"
    //"##hgtest"
};

static char user_entry [IRC_MESSAGE_MAX + 1];
static unsigned long long last_ping_in_us;
static const int delay = 14985;

struct linked_list *
new_buffer (char *);

signed
handle_server_message (struct linked_list *, signed, char *);

void
signal_handler (signed);

#define C(x) ((x) & 0x1F)

#endif
