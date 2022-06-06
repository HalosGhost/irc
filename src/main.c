#include "main.h"

signed
main (void) {

    xxhashmap_init(channels, 5);

    setlocale(LC_ALL, "");

    initscr();
    nodelay(stdscr, true);
    noecho();
    cbreak();
    keypad(stdscr, true);

    refresh();
    WINDOW * statbar = newwin(1, 0, LINES - 2, 0);
    wattron(statbar, A_REVERSE);
    WINDOW * inputln = newwin(1, 0, LINES - 1, 0);

    struct linked_list * serv = new_buffer(server);
    chan = serv;

    size_t autojoin_sz = sizeof autojoin / sizeof *autojoin;
    for ( size_t i = 0; i < autojoin_sz; ++i ) {
        new_buffer(autojoin[i]);
    }

    signal(SIGINT, signal_handler);

    signed fd = irc_connect(server, port);
    if ( fd < 0 ) {
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    struct pollfd pfd[1] = {
        { .fd = fd, .events = POLLIN }
    };

    signed cmd_status = EXIT_SUCCESS;

    cmd_status = irc_authenticate(fd, nick, ident, gecos, pass);
    if ( cmd_status != EXIT_SUCCESS ) {
        exit_status = cmd_status;
        goto cleanup;
    }

    bool running = true;
    do {
        wint_t ch;
        signed res = get_wch(&ch);
        clock_t mark = clock();
        // todo: try to figure out horizontal-scrolling for the input window
        static size_t user_entry_len = 0;
        if ( res != ERR ) {
            werase(inputln);

            switch ( ch ) {
                case '\n':
                    if ( user_entry_len ) {
                        enum cmd_builtin st = C_MESSAGE;
                        if ( user_entry[0] == '/' && user_entry[1] != '/' ) {
                            st = identify_cmd(user_entry + 1);
                        }
                        enum irc_command cmd = builtin_irc[st];

                        switch ( st ) {
                            case C_QUIT:
                                running = false;
                                continue;

                            // todo: C_CONNECT should be about the same
                            case C_QUERY:
                            case C_JOIN: {
                                char * token = strtok(user_entry, " \r\n"); // skip cmd
                                while ( (token = strtok(NULL, " \r\n")) ) {
                                    chan = new_buffer(token);
                                    if ( st == C_JOIN ) {
                                        irc_join(fd, token);
                                    }
                                }
                            } break;

                            case C_ACTION: {
                                wprintw(chan->buf.win, "%s %s\n", nick, user_entry + sizeof "/me");
                                size_t newsize = user_entry_len - 4 + sizeof "ACTION ";
                                char * tmpmsg = malloc(newsize);
                                snprintf(tmpmsg, newsize, "ACTION %s", user_entry + sizeof "/me");
                                irc_send(fd, cmd, chan->name, tmpmsg);
                                fprintf(chan->buf.log, ":%s!~%s@localhost ", nick, ident);
                                fprintf(chan->buf.log, irc_command_fmt[cmd], chan->name, tmpmsg);
                                fputs("\r\n", chan->buf.log);
                                ring_insert(chan->buf.hist, C_ACTION, nick, user_entry + sizeof "/me");
                                free(tmpmsg);
                                wnoutrefresh(chan->buf.win);
                                wnoutrefresh(statbar);
                            } break;

                            case C_MESSAGE: {
                                irc_send(fd, cmd, chan->name, user_entry + (user_entry[0] == '/'));
                                wprintw(chan->buf.win, "<%s> %s\n", nick, user_entry);
                                fprintf(chan->buf.log, ":%s!~%s@localhost ", nick, ident);
                                fprintf(chan->buf.log, irc_command_fmt[cmd], chan->name, user_entry + (user_entry[0] == '/'));
                                fputs("\r\n", chan->buf.log);
                                ring_insert(chan->buf.hist, C_MESSAGE, nick, user_entry);
                                wnoutrefresh(chan->buf.win);
                                wnoutrefresh(statbar);
                            } break;

                            case C_TOPIC:
                            case C_NAMES: {
                                char * token = strtok(user_entry, " \r\n");
                                while ( true ) {
                                    if ( !(token = strtok(NULL, " \r\n")) ) {
                                        irc_send(fd, builtin_irc[st], chan->name);
                                    } else {
                                        irc_send(fd, builtin_irc[st], token);
                                    } break;
                                }
                                wnoutrefresh(statbar);
                            } break;

                            case C_BUFFERS:
                                for ( size_t b = 0, i = 0; b < 1u << channels->capacity; ++b ) {
                                    struct linked_list * cb = channels->buckets[b];
                                    while ( cb ) {
                                        wprintw(chan->buf.win, "[%zu] %s\t\t", i, cb->name);
                                        ++i;
                                        cb = cb->next;
                                    }
                                }
                                wprintw(chan->buf.win, "\n");
                                wnoutrefresh(chan->buf.win);
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
                                werase(chan->buf.win);
                                wmove(chan->buf.win, 0, 0);
                                for ( signed i = 1; i <= LINES - 2; ++i ) {
                                    char * line = ring_get(chan->buf.hist, i);
                                    if ( line ) {
                                        signed len = strlen(line);
                                        wprintw(chan->buf.win, "%*s", len, line);
                                    } else {
                                        wprintw(chan->buf.win, "\r\n");
                                    }
                                }
                                wnoutrefresh(chan->buf.win);
                                wnoutrefresh(statbar);
                            } break;

                            default:
                            case C_UNKNOWN:
                                wprintw(chan->buf.win, "unknown command: %s\n", user_entry);
                                wnoutrefresh(chan->buf.win);
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

                // todo: handle SIGWINCH
                case KEY_RESIZE: break;

                default:
                    // todo: fix multi-byte grapheme clusters
                    if ( user_entry_len < 512 ) {
                        user_entry_len += sprintf(user_entry + user_entry_len, "%lc", ch);
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
                    fprintf(serv->buf.log, "read() failed: %s\n", strerror(errsv));
                    exit_status = EXIT_FAILURE;
                    goto cleanup;
                }
            } else if ( !bytes_read ) {
                fputs("connection closed\n", serv->buf.log);
                goto cleanup;
            }

            static bool joined = false;
            if ( !*servername ) {
                sscanf(msg_buf, "%s NOTICE", servername);
            } else if ( !joined ) {
                cmd_status = irc_joinall(fd, (sizeof autojoin / sizeof *autojoin), autojoin);
                joined = true;
            }

            handle_server_message(serv, fd, msg_buf);
            wnoutrefresh(chan->buf.win);
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
                delwin(l->buf.win);
                fclose(l->buf.log);
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

struct linked_list *
new_buffer (char * key) {

    WINDOW * buf = newwin(LINES - 1, 0, 0, 0);
    scrollok(buf, true);
    FILE * log = fopen(key, "a"); // "a+" with fseek dance for scrollback
    setlinebuf(log);
    xxhashmap_insert(channels, key, buf, log);
    return xxhashmap_get(channels, key);
}

signed
handle_server_message (struct linked_list * serv, signed filedes, char * message) {

    signed cmd_status;

    char s_handle [IRC_MESSAGE_MAX] = { 0 };
    char s_host [IRC_MESSAGE_MAX] = { 0 };
    char s_mask [IRC_MESSAGE_MAX] = { 0 };
    char s_chan [IRC_MESSAGE_MAX] = { 0 };
    char s_msg [IRC_MESSAGE_MAX] = { 0 };
    signed matched = sscanf(message, ":%[^!]!%[^@]@%s PRIVMSG %s %[^\n]",
        s_handle, s_host, s_mask, s_chan, s_msg);
    if ( matched == 5 ) {
        char * tgt = !strcmp(s_chan, nick) ? s_handle : s_chan;
        if ( !xxhashmap_contains(channels, tgt) ) {
            new_buffer(tgt);
        }
        struct linked_list * ch = xxhashmap_get(channels, tgt);
        WINDOW * b = ch->buf.win;
        char s_act [IRC_MESSAGE_MAX] = { 0 };
        matched = sscanf(s_msg + (s_msg[0] == ':'), "ACTION %[^]", s_act);
        if ( matched == 1 ) {
            wprintw(b, "%s %s\n", s_handle, s_act);
            ring_insert(ch->buf.hist, C_ACTION, s_handle, s_act);
        } else {
            // prevent \r\n from clearing the line
            // todo: find more robustly with strstr
            for ( size_t i = 0; i < IRC_MESSAGE_MAX; ++i ) {
                if ( s_msg[i] == '\r' ) { s_msg[i] = ' '; }
            }
            wprintw(b, "<%s> %s\n", s_handle, s_msg + (s_msg[0] == ':'));
            ring_insert(ch->buf.hist, C_MESSAGE, s_handle, s_msg + (s_msg[0] == ':'));
        }
        fprintf(ch->buf.log, "%s", message);
    } else if ( !strncmp(message, "PING", 4) ) {
        last_ping_in_us = 0;
        cmd_status = irc_send(filedes, PONG, servername);
        if ( cmd_status == EXIT_FAILURE ) {
            return EXIT_FAILURE;
        }
    } else {
        fprintf(serv->buf.log, "%s", message);
        // prevent \r\n from clearing the line
        // todo: find more robustly with strstr
        for ( size_t i = 0; i < IRC_MESSAGE_MAX; ++i ) {
            if ( message[i] == '\r' ) { message[i] = ' '; }
        }
        wprintw(serv->buf.win, "%s", message);
        ring_insert(serv->buf.hist, C_UNKNOWN, message);
    }

    return EXIT_SUCCESS;
}

void
signal_handler (signed signum) {

    caught_signum = (sig_atomic_t )signum;
}

