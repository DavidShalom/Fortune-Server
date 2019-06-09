//
//  Solution by: Shalom-David Anifowoshe
//
//

#ifndef _TCP_FUNCTIONS_H
#define _TCP_FUNCTIONS_H

#include <unistd.h>

extern int tcp_listen(const char *host, const char *port);

extern ssize_t tcp_read(int fd, void *buff, size_t count);

extern ssize_t tcp_write(int fd, const void *buff, size_t count);

extern int tcp_accept(int fd);

extern int tcp_close(int fd);

#endif
