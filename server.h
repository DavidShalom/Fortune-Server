//
//  Solution by: Shalom-David Anifowoshe
//
//

#ifndef _SERVER_H
#define _SERVER_H

#define _POSIX_C_SOURCE 200809L
#define _DARWIN_C_SOURCE
#define _BSD_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <netdb.h>
#include <event2/event.h>

#include "tcp_functions.h"
#include "structures.h"
typedef enum { UNKNOWN, GENERIC, CHALLENGE, OK, FAIL } reply_type;

static void server_to_client(tcp_node *tcp_connection, reply_type rt, char* generic_msg);

static void callback(evutil_socket_t evfd, short evwhat, void *evarg);

static void get_fortune(tcp_node *tcp_connection);

static void read_fortune(evutil_socket_t evfd, short evwhat, void *evarg);

static void read_client_message(evutil_socket_t evfd, short evwhat, void *evarg);

static void replace_word(game_node* status);



#endif
