#include "main.h"

signed
main (void) {

    xxhashmap_init(channels, 5);

    initscr();
    nodelay(stdscr, true);
    noecho();
    cbreak();
    keypad(stdscr, true);

    refresh();
    WINDOW * statbar = newwin(1, 0, LINES - 2, 0);
    wattron(statbar, A_REVERSE);
    WINDOW * inputln = newwin(1, 0, LINES - 1, 0);

    FILE * logfile = fopen(server, "a");
    if ( !logfile ) {
        fprintf(stderr, "fopen() failed to open log file (%s): %s\n", server, strerror(errno));
        return EXIT_FAILURE;
    }
    setlinebuf(logfile);
    WINDOW * serv = newwin(LINES - 1, 0, 0, 0);
    scrollok(serv, true);
    xxhashmap_insert(channels, server, serv, logfile);

    // todo: move autojoin-buffer creation into the channel joining
    size_t autojoin_sz = sizeof autojoin / sizeof *autojoin;
    for ( size_t i = 0; i < autojoin_sz; ++i ) {
        WINDOW * newbuf = newwin(LINES - 1, 0, 0, 0);
        scrollok(newbuf, true);
        FILE * log = fopen(autojoin[i], "a"); // "a+" with fseek dance for scrollback
        setlinebuf(log);
        xxhashmap_insert(channels, autojoin[i], newbuf, log);
    }
    for ( size_t i = 0; i < 1u << channels->capacity; ++i ) {
        chan = channels->buckets[i];
        if ( chan ) { break; }
    }

    signal(SIGINT, signal_handler);

    signed fd = irc_connect(logfile, server, port);
    if ( fd < 0 ) {
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    struct pollfd pfd[1] = {
        { .fd = fd, .events = POLLIN }
    };

    signed cmd_status = EXIT_SUCCESS;

    cmd_status = irc_authenticate(logfile, fd, nick, ident, gecos, pass);
    if ( cmd_status != EXIT_SUCCESS ) {
        exit_status = cmd_status;
        goto cleanup;
    }

    bool running = true;
    do {
        signed ch = getch();
        clock_t mark = clock();
        // todo: try to figure out horizontal-scrolling for the input window
        static size_t user_entry_len = 0;
        if ( ch != ERR ) {
            werase(inputln);

            switch ( ch ) {
                case '\n':
                    if ( user_entry_len ) {
                        enum cmd_builtin st;
                        if ( user_entry[0] == '/' && user_entry[1] != '/' ) {
                            st = identify_cmd(user_entry + 1);
                        } else {
                            st = C_MESSAGE;
                        }

                        switch ( st ) {
                            case C_QUIT:
                                running = false;
                                continue;

                            case C_JOIN: {
                                char * token = strtok(user_entry, " \r\n"); // skip "/join"
                                while ( (token = strtok(NULL, " \r\n")) ) {
                                    WINDOW * newbuf = newwin(LINES - 1, 0, 0, 0);
                                    scrollok(newbuf, true);
                                    FILE * log = fopen(token, "a"); // "a+" with fseek dance for scrollback
                                    setlinebuf(log);
                                    xxhashmap_insert(channels, token, newbuf, log);
                                    chan = xxhashmap_get(channels, token);
                                    irc_join(chan->log, fd, token);
                                }
                            } break;

                            case C_ACTION: {
                                wprintw(chan->buf, "%s %s\n", nick, user_entry + sizeof "/me");
                                size_t newsize = user_entry_len - 4 + sizeof "ACTION ";
                                char * tmpmsg = malloc(newsize);
                                snprintf(tmpmsg, newsize, "ACTION %s", user_entry + 4);
                                irc_send(chan->log, fd, PRIVMSG, chan->name, tmpmsg);
                                fprintf(chan->log, ":%s!~%s@localhost ", nick, ident);
                                fprintf(chan->log, irc_command_fmt[PRIVMSG], chan->name, tmpmsg);
                                fputs("\r\n", chan->log);
                                free(tmpmsg);
                                wnoutrefresh(chan->buf);
                                wnoutrefresh(statbar);
                            } break;

                            case C_MESSAGE: {
                                irc_send(chan->log, fd, PRIVMSG, chan->name, user_entry + (user_entry[0] == '/'));
                                wprintw(chan->buf, "<%s> %s\n", nick, user_entry);
                                fprintf(chan->log, ":%s!~%s@localhost ", nick, ident);
                                fprintf(chan->log, irc_command_fmt[PRIVMSG], chan->name, user_entry + (user_entry[0] == '/'));
                                fputs("\r\n", chan->log);
                                wnoutrefresh(chan->buf);
                                wnoutrefresh(statbar);
                            } break;

                            case C_BUFFERS:
                                for ( size_t b = 0, i = 0; b < 1u << channels->capacity; ++b ) {
                                    struct linked_list * cb = channels->buckets[b];
                                    while ( cb ) {
                                        wprintw(chan->buf, "[%zu] %s\t\t", i, cb->name);
                                        ++i;
                                        cb = cb->next;
                                    }
                                }
                                wprintw(chan->buf, "\n");
                                wnoutrefresh(chan->buf);
                                wnoutrefresh(statbar);
                                break;

                            case C_GOTO: {
                                size_t tgt;
                                sscanf(user_entry + sizeof "/goto", "%zu", &tgt);
                                for ( size_t b = 0, i = 0; b < 1u << channels->capacity; ++b ) {
                                    struct linked_list * cb = channels->buckets[b];
                                    while ( cb ) {
                                        if ( i == tgt ) {
                                            chan = cb;
                                            break;
                                        }
                                        ++i;
                                        cb = cb->next;
                                    }
                                    if ( cb ) { break; }
                                }
                                wnoutrefresh(chan->buf);
                                wnoutrefresh(statbar);
                            } break;

                            default:
                            case C_UNKNOWN:
                                wprintw(chan->buf, "unknown command: %s\n", user_entry);
                                wnoutrefresh(chan->buf);
                                wnoutrefresh(statbar);
                                break;
                        }

                        memset(user_entry, 0, IRC_MESSAGE_MAX);
                        user_entry_len = 0;
                    }
                    break;

                case 127:
                case KEY_BACKSPACE:
                case KEY_DC:
                    if ( user_entry_len ) {
                        user_entry[--user_entry_len] = 0;
                    }
                    break;

                case C('u'):
                    memset(user_entry, 0, IRC_MESSAGE_MAX);
                    user_entry_len = 0;
                    break;

                default:
                    if ( user_entry_len < 512 ) {
                        user_entry[user_entry_len++] = ch;
                    }
                    break;
            }

            mvwprintw(inputln, 0, 0, "%s", user_entry);
            wnoutrefresh(inputln);
        }

        signed pres = poll(pfd, 1, 0);
        if ( pres < 0 ) {
            running = false;
            continue;
        }

        if ( exit_status == EXIT_FAILURE ) {
            goto cleanup;
        }

        if ( pres == -1 ) {
            if ( errno == EAGAIN ) {
                continue;
            }

            exit_status = EXIT_FAILURE;
            goto cleanup;
        }

        static char msg_buf [IRC_MESSAGE_MAX + 1] = "";

        if ( pfd[0].revents & POLLIN ) {
            memset(msg_buf, 0, IRC_MESSAGE_MAX);

            signed errsv = errno = 0;
            ssize_t bytes_read = read(fd, msg_buf, IRC_MESSAGE_MAX);

            if ( bytes_read == -1 ) {
                errsv = errno;
                if ( errsv == EAGAIN || errsv == EWOULDBLOCK ) {
                    continue;
                } else {
                    fprintf(logfile, "read() failed: %s\n", strerror(errsv));
                    exit_status = EXIT_FAILURE;
                    goto cleanup;
                }
            } else if ( !bytes_read ) {
                fputs("connection closed\n", logfile);
                goto cleanup;
            }

            static bool joined = false;
            if ( !*servername ) {
                sscanf(msg_buf, "%s NOTICE", servername);
            } else if ( !joined ) {
                cmd_status = irc_joinall(logfile, fd, (sizeof autojoin / sizeof *autojoin), autojoin);
                joined = true;
            }

            // prevent \r\n from clearing the line
            for ( size_t i = 0; i < IRC_MESSAGE_MAX; ++i ) {
                if ( msg_buf[i] == '\r' ) { msg_buf[i] = ' '; }
            }

            char s_handle [IRC_MESSAGE_MAX] = { 0 };
            char s_host [IRC_MESSAGE_MAX] = { 0 };
            char s_mask [IRC_MESSAGE_MAX] = { 0 };
            char s_chan [IRC_MESSAGE_MAX] = { 0 };
            char s_msg [IRC_MESSAGE_MAX] = { 0 };
            signed matched = sscanf(msg_buf, ":%[^!]!%[^@]@%s PRIVMSG %s %[^\n]",
                s_handle, s_host, s_mask, s_chan, s_msg);
            if ( matched == 5 ) {
                struct linked_list * ch = xxhashmap_get(channels, s_chan);
                WINDOW * b = ch->buf;
                char s_act [IRC_MESSAGE_MAX] = { 0 };
                matched = sscanf(s_msg + (s_msg[0] == ':'), "ACTION %[^]", s_act);
                if ( matched == 1 ) {
                    wprintw(b, "%s %s\n", s_handle, s_act);
                } else {
                    wprintw(b, "<%s> %s\n", s_handle, s_msg + (s_msg[0] == ':'));
                }
                fprintf(ch->log, "%s", msg_buf);
            } else if ( strncmp(msg_buf, "PING", 4) ) {
                wprintw(serv, "%s", msg_buf);
                fprintf(logfile, "%s", msg_buf);
            }
            handle_server_message(logfile, fd, msg_buf);
            wnoutrefresh(chan->buf);
        }

        last_ping_in_us += delay;
        signed spacer = COLS - (strlen(chan->name) + 2);
        mvwprintw(statbar, 0, 0, "%s%*llus ", chan->name, spacer, last_ping_in_us / 1000000);
        wmove(inputln, 0, user_entry_len);
        wnoutrefresh(statbar);
        wnoutrefresh(inputln);

        doupdate();
        errno = 0;

        clock_t elapsed = clock() - mark;
        if ( delay > elapsed ) {
            usleep(delay - elapsed);
        }
    } while ( running );

    cleanup:
        for ( size_t i = 0; i < 1u << channels->capacity; ++i ) {
            struct linked_list * l = channels->buckets[i];
            while ( l ) {
                delwin(l->buf);
                fclose(l->log);
                l = l->next;
            }
        }
        if ( statbar ) { delwin(statbar); }
        if ( inputln ) { delwin(inputln); }
        if ( fd > 0 ) { close(fd); }
        if ( channels ) { xxhashmap_free(channels); }
        endwin();

        return exit_status;
}

signed
handle_server_message (FILE * logfile, signed filedes, char * message) {

    signed cmd_status;

    if ( !strncmp(message, "PING", 4) ) {
        last_ping_in_us = 0;
        cmd_status = irc_send(logfile, filedes, PONG, servername);
        if ( cmd_status == EXIT_FAILURE ) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void
signal_handler (signed signum) {

    caught_signum = (sig_atomic_t )signum;
}

