#include "main.h"

signed
main (void) {

    signed fd = irc_connect(server, port);
    if ( fd < 0 ) {
        return EXIT_FAILURE;
    }

    struct pollfd pfd[1] = {
        { .fd = fd, .events = POLLIN }
    };

    signed cmd_status;

    initscr(); noecho(); cbreak(); keypad(stdscr, true);
    mvhline(LINES - 2, 0, 0, COLS);
    refresh();
    WINDOW * buffer = newwin(LINES - 2, 0, 0, 0);
    WINDOW * inputln = newwin(1, 0, LINES - 1, 0);
    scrollok(buffer, true);


    signal(SIGINT, signal_handler);

    cmd_status = irc_send(fd, NICK, nick);
    if ( cmd_status == EXIT_FAILURE ) {
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    cmd_status = irc_send(fd, USER, ident ? ident : nick, gecos ? gecos : nick);
    if ( cmd_status == EXIT_FAILURE ) {
        exit_status = EXIT_FAILURE;
        goto cleanup;
    }

    for ( signed pres = poll(pfd, 1, -1); pres; pres = poll(pfd, 1, -1) ) {
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

            // prevent \r\n from clearing the line
            for ( size_t i = 0; i < IRC_MESSAGE_MAX; ++i ) {
                if ( msg_buf[i] == '\r' ) { msg_buf[i] = ' '; }
            }

            if ( bytes_read == -1 ) {
                errsv = errno;
                if ( errsv == EAGAIN || errsv == EWOULDBLOCK ) {
                    continue;
                } else {
                    fprintf(stderr, "read() failed: %s\n", strerror(errsv));
                    exit_status = EXIT_FAILURE;
                    goto cleanup;
                }
            } else if ( !bytes_read ) {
                fputs("connection closed\n", stderr);
                goto cleanup;
            }

            static bool joined = false;
            if ( !*servername ) {
                sscanf(msg_buf, "%s NOTICE", servername);
            } else if ( !joined ) {
                size_t len = sizeof(channels) / sizeof(*channels);
                size_t i = 0;
                while ( i < len ) {
                    irc_send(fd, JOIN, channels[i++]);
                }

                joined = true;
            }

            wprintw(buffer, "%s", msg_buf);
            handle_server_message(fd, msg_buf);
            wrefresh(buffer);
        }

        errno = 0;
    }

    cleanup:
        if ( buffer ) { delwin(buffer); }
        if ( inputln ) { delwin(inputln); }
        if ( fd > 0 ) { close(fd); }
        endwin();

        return EXIT_SUCCESS;
}

signed
handle_server_message (signed filedes, char * message) {

    signed cmd_status;

    if ( !strncmp(message, "PING", 4) ) {
        cmd_status = irc_send(filedes, PONG, servername);
        if ( cmd_status == EXIT_FAILURE ) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void
signal_handler (signed signum) {

    caught_signum = (sig_atomic_t )signum;
    exit_status = EXIT_FAILURE;
}

