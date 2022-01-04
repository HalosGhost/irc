#include "main.h"

signed
main (void) {

    FILE * logfile = fopen(logpath, "a");
    if ( !logfile ) {
        fprintf(stderr, "fopen() failed to open log file (%s): %s\n", logpath, strerror(errno));
        return EXIT_FAILURE;
    }

    setlinebuf(logfile);

    initscr();
    nodelay(stdscr, true);
    noecho();
    cbreak();
    keypad(stdscr, true);

    refresh();
    WINDOW * statbar = newwin(1, 0, LINES - 2, 0);
    wattron(statbar, A_REVERSE);
    WINDOW * buffer = newwin(LINES - 1, 0, 0, 0);
    WINDOW * inputln = newwin(1, 0, LINES - 1, 0);
    scrollok(buffer, true);

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
        static size_t user_entry_len = 0;
        if ( ch != ERR ) {
            werase(inputln);

            switch ( ch ) {
                case '\n':
                    if ( user_entry_len ) {
                        enum cmd_builtin st = handle_local_message(logfile, fd, user_entry);
                        switch ( st ) {
                            case C_QUIT:
                                running = false;
                                continue;

                            case C_JOIN: {
                                char * token = strtok(user_entry, " \r\n"); // skip "/join"
                                while ( (token = strtok(NULL, " \r\n")) ) {
                                    wprintw(buffer, "joining %s\n", token);
                                    irc_join(logfile, fd, token);
                                }
                            } break;

                            case C_MESSAGE:
                                wprintw(buffer, "%s\n", user_entry);
                                wnoutrefresh(buffer);
                                wnoutrefresh(statbar);
                                break;

                            default:
                            case C_UNKNOWN:
                                wprintw(buffer, "unknown command: %s\n", user_entry);
                                wnoutrefresh(buffer);
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
                cmd_status = irc_joinall(logfile, fd, (sizeof channels / sizeof *channels), channels);
                joined = true;
            }

            // prevent \r\n from clearing the line
            for ( size_t i = 0; i < IRC_MESSAGE_MAX; ++i ) {
                if ( msg_buf[i] == '\r' ) { msg_buf[i] = ' '; }
            }

            if ( strncmp(msg_buf, "PING", 4) ) {
                wprintw(buffer, "%s", msg_buf);
            }
            fprintf(logfile, "%s", msg_buf);
            handle_server_message(logfile, fd, msg_buf);
            wnoutrefresh(buffer);
        }

        last_ping_in_us += delay;
        mvwprintw(statbar, 0, 0, "%*llus ", COLS - 2, last_ping_in_us / 1000000);
        wmove(inputln, 0, user_entry_len);
        wnoutrefresh(statbar);
        wnoutrefresh(inputln);

        doupdate();
        errno = 0;
        usleep(delay);
    } while ( running );

    cleanup:
        if ( buffer ) { delwin(buffer); }
        if ( statbar ) { delwin(statbar); }
        if ( inputln ) { delwin(inputln); }
        if ( fd > 0 ) { close(fd); }
        if ( logfile ) { fclose(logfile); }
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

enum cmd_builtin
handle_local_message (FILE * logfile, signed filedes, char * message) {

    if ( message[0] == '/' && message[1] != '/' ) {
        return identify_cmd(message + 1);
    } else {
        signed cmd_status = irc_send(logfile, filedes, PRIVMSG, channels[0], message + (message[0] == '/'));
        return cmd_status == EXIT_SUCCESS ? C_MESSAGE : C_UNKNOWN;
    }

    return C_UNKNOWN;
}

void
signal_handler (signed signum) {

    caught_signum = (sig_atomic_t )signum;
}

