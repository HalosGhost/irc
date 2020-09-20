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
irc_send (signed filedes, enum irc_command cmd, ...) {

    va_list args;
    va_start(args, cmd);

    signed errsv;

    static char msg_buf [IRC_MESSAGE_MAX + 1];
    signed length = irc_cmdf(cmd, msg_buf, args);

    #if !defined(NDEBUG)
        fprintf(stderr, "sending %s", msg_buf);
    #endif

    errsv = errno = 0;
    ssize_t bytes_written = write(filedes, msg_buf, (size_t )length);
    if ( bytes_written < 0 ) {
        errsv = errno;
        fprintf(stderr, "write() failed: %s\n", strerror(errsv));
        return EXIT_FAILURE;
    }

    memset(msg_buf, 0, IRC_MESSAGE_MAX);

    va_end(args);

    return EXIT_SUCCESS;
}

signed
irc_connect (char * server, char * port) {

    memset(servername, 0, IRC_MESSAGE_MAX);

    struct addrinfo hints;
    struct addrinfo * res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    signed status = getaddrinfo(server, port, &hints, &res);
    if ( status ) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    signed errsv = errno = 0;
    signed fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if ( fd < 0 ) {
        errsv = errno;
        fprintf(stderr, "socket() failed: %s\n", strerror(errsv));

        freeaddrinfo(res);
        return -1;
    }

    errsv = errno = 0;
    status = connect(fd, res->ai_addr, res->ai_addrlen);
    if ( status < 0 ) {
        errsv = errno;
        fprintf(stderr, "connect() failed: %s\n", strerror(errsv));

        close(fd);
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);

    errsv = errno = 0;
    status = fcntl(fd, F_SETFL, O_NONBLOCK);
    if ( status < 0 ) {
        errsv = errno;
        fprintf(stderr, "fcntl() failed: %s\n", strerror(errsv));

        close(fd);
        return -1;
    }

    return fd;
}
