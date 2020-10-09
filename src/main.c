#include "main.h"

signed
main (void) {

    FILE * logfile = fopen(logpath, "a");
    if ( !logfile ) {
        fprintf(stderr, "fopen() failed to open log file (%s): %s\n", logpath, strerror(errno));
        return EXIT_FAILURE;
    }

    initscr(); noecho(); cbreak(); keypad(stdscr, true);
    mvhline(LINES - 2, 0, 0, COLS);
    refresh();
    WINDOW * buffer = newwin(LINES - 2, 0, 0, 0);
    WINDOW * inputln = newwin(1, 0, LINES - 1, 0);
    scrollok(buffer, true);

    signal(SIGINT, signal_handler);

    signed fd = irc_connect(logfile, server, port);
    if ( fd < 0 ) {
        return EXIT_FAILURE;
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

            wprintw(buffer, "%s", msg_buf);
            handle_server_message(logfile, fd, msg_buf);
            wrefresh(buffer);
        }

        errno = 0;
    }

    cleanup:
        if ( buffer ) { delwin(buffer); }
        if ( inputln ) { delwin(inputln); }
        if ( fd > 0 ) { close(fd); }
        if ( logfile ) { fclose(logfile); }
        endwin();

        return EXIT_SUCCESS;
}

signed
handle_server_message (FILE * logfile, signed filedes, char * message) {

    signed cmd_status;

    if ( !strncmp(message, "PING", 4) ) {
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
    exit_status = EXIT_FAILURE;
}

