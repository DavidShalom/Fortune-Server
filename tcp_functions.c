//
//  Solution by: Shalom-David Anifowoshe
//
//


#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>

#include "tcp_functions.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST      1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV      32
#endif

int tcp_listen(const char *host, const char *port)
{
    int fd = 0, on = 1;
    struct addrinfo hints, *ai_list, *ai;

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int n = getaddrinfo(host, port, &hints, &ai_list);
    if (n)
    {
        syslog(LOG_ERR, "getaddrinfo failed: %s", gai_strerror(n));
        return -1;
    }

    for (ai = ai_list; ai; ai = ai->ai_next)
    {
        fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (fd < 0) continue;


#ifdef IPV6_V6ONLY

        /*
         * Some IPv6 stacks by default accept IPv4-mapped addresses on
         * IPv6 sockets and hence binding a port separately for both
         * IPv4 and IPv6 sockets fails on these systems by default.
         * This can be avoided by making the IPv6 socket explicitly an
         * IPv6 only socket.
         */

        if (ai->ai_family == AF_INET6)
        {
            (void) setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
        }
#endif

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if (bind(fd, ai->ai_addr, ai->ai_addrlen) == 0)
         {
            break;
         }
        close(fd);
     }


    freeaddrinfo(ai_list);

    if (ai == NULL)
    {
        syslog(LOG_ERR, "bind failed for port %s", port);
        return -1;
    }

    if (listen(fd, 42) < 0)
    {
        syslog(LOG_ERR, "listen failed: %s", strerror(errno));
        close(fd);
        return -1;
    }

    return fd;
}

ssize_t tcp_read(int fd, void *buf, size_t count)
{
    size_t nread = 0;
    size_t r;

    while (count > 0)
    {
        r = (int) read(fd, buf, count);
        if (r < 0 && errno == EINTR) continue;
        if (r < 0) return r;
        if (r == 0) return nread;

        buf = (unsigned char *) buf + r;
        count -= r;
        nread += r;
    }

    return nread;
}

ssize_t tcp_write(int fd, const void *buf, size_t count)
{
    size_t nwritten = 0;
    size_t r;

    while (count > 0)
    {
        r = write(fd, buf, count);
        if (r < 0 && errno == EINTR) continue;
        if (r < 0) return r;
        if (r == 0) return nwritten;

        buf = (unsigned char *) buf + r;
        count -= r;
        nwritten += r;
    }

    return nwritten;
}

int tcp_accept(int listen)
{
    char host[NI_MAXHOST], server[NI_MAXSERV];
    struct sockaddr_storage ss;
    socklen_t ss_length = sizeof(ss);


    //accepts connection
    int fd = accept(listen, (struct sockaddr *) &ss, &ss_length);
    if (fd == -1) {
        syslog(LOG_ERR, "accept failed: %s", strerror(errno));
        return -1;
    }

    int n = getnameinfo((struct sockaddr *) &ss, ss_length, host, sizeof(host), server, sizeof(server), NI_NUMERICHOST);

    //write about what was accepted
    if (n) {
        syslog(LOG_ERR, "getnameinfo failed: %s", gai_strerror(n));
    } else {
        syslog(LOG_DEBUG, "connection from %s%s%s:%s", strchr(host, ':') == NULL ? "" : "[", host, strchr(host, ':') == NULL ? "" : "]", server);
    }

    return fd;
}

int tcp_close(int fd)
{
    return close(fd); //calls close() to close tcp connection
}
