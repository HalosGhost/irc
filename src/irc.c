#include "irc.h"

signed
irc_cmdf (enum irc_command cmd, char * str, va_list args) {

    signed bytes_written = (!!str) - 1;
    if ( bytes_written < 0 ) {
        return bytes_written;
    }

    const char * fmt = irc_command_fmt[cmd];

    bytes_written = vsnprintf(str, IRC_MESSAGE_MAX - 2, fmt, args);
    if ( bytes_written < 0 ) {
        return bytes_written;
    }

    str[bytes_written++] = IRC_LINE_ENDING[0];
    str[bytes_written++] = IRC_LINE_ENDING[1];
    str[bytes_written + 1] = IRC_LINE_ENDING[2];

    return bytes_written;
}

signed
irc_send (FILE * logfile, BIO * bio, enum irc_command cmd, ...) {

    assert(logfile);

    va_list args;
    va_start(args, cmd);

    signed errsv;

    static char msg_buf [IRC_MESSAGE_MAX + 1];
    irc_cmdf(cmd, msg_buf, args);

    fprintf(logfile, "%s", msg_buf);

    ssize_t bytes_written = BIO_printf(bio, "%s", msg_buf);
    if ( bytes_written < 0 ) {
        fprintf(logfile, "%s() failed\n", __func__);
        return EXIT_FAILURE;
    }

    memset(msg_buf, 0, IRC_MESSAGE_MAX);

    va_end(args);

    return EXIT_SUCCESS;
}

signed
irc_authenticate (FILE * logfile, BIO * bio, char * nick, char * ident, char * gecos, char * pass) {

    signed cmd_status = EXIT_SUCCESS;
    if ( pass ) {
        cmd_status = irc_send(logfile, bio, PASS, pass);
        if ( cmd_status != EXIT_SUCCESS ) {
            return cmd_status;
        }
    }

    cmd_status = irc_send(logfile, bio, NICK, nick);
    if ( cmd_status != EXIT_SUCCESS ) {
        return cmd_status;
    }

    return irc_send(logfile, bio, USER, ident ? ident : nick, gecos ? gecos : nick);
}

signed
irc_joinall(FILE * logfile, BIO * bio, size_t num_channels, char * channels[]) {

    signed cmd_status = EXIT_SUCCESS;
    for ( size_t i = 0; i < num_channels; ++i ) {
        cmd_status = irc_send(logfile, bio, JOIN, channels[i]);
        if ( cmd_status != EXIT_SUCCESS ) {
            return cmd_status;
        }
    }

    return cmd_status;
}

BIO *
irc_connect (FILE * logfile, char * server, char * port) {

    assert(logfile);

//    struct addrinfo hints;
//    struct addrinfo * res;
//
//    memset(&hints, 0, sizeof(struct addrinfo));
//    hints.ai_family = AF_UNSPEC;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_flags = AI_PASSIVE;
//    
//    signed status = getaddrinfo(server, port, &hints, &res);
//    if ( status ) {
//        fprintf(logfile, "getaddrinfo() failed: %s\n", gai_strerror(status));
//        return EXIT_FAILURE;
//    }
//
//    signed errsv = errno = 0;
//    signed fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//    if ( fd < 0 ) {
//        errsv = errno;
//        fprintf(logfile, "socket() failed: %s\n", strerror(errsv));
//
//        freeaddrinfo(res);
//        return -1;
//    }

    BIO * bio = BIO_new_connect(server);
    if ( !bio ) {
        fprintf(logfile, "BIO_new_connect() failed\n");
        return NULL;
    }

    BIO_set_conn_port(bio, port);
    //BIO_set_conn_ip_family(bio, AF_UNSPEC);
    BIO_set_nbio(bio, 1);
    signed rv = 0;
    do {
        sleep(1);
        rv = BIO_do_connect(bio);
    } while ( rv < 1 && BIO_should_retry(bio) );

    if ( rv < 1 ) {
        unsigned long long errsv = ERR_get_error();
        fprintf(logfile, "BIO_do_connect() failed: %s\n", ERR_error_string(errsv, NULL));
        return NULL;
    }

    return bio;
//
//    errsv = errno = 0;
//    status = connect(fd, res->ai_addr, res->ai_addrlen);
//    if ( status < 0 ) {
//        errsv = errno;
//        fprintf(logfile, "connect() failed: %s\n", strerror(errsv));
//
//        close(fd);
//        freeaddrinfo(res);
//        return -1;
//    }
//
//    freeaddrinfo(res);
//
//    errsv = errno = 0;
//    status = fcntl(fd, F_SETFL, O_NONBLOCK);
//    if ( status < 0 ) {
//        errsv = errno;
//        fprintf(logfile, "fcntl() failed: %s\n", strerror(errsv));
//
//        close(fd);
//        return -1;
//    }
//
//    return fd;
}
