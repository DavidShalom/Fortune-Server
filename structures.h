//
//  Solution by: Shalom-David Anifowoshe
//
//

#ifndef _STRUCTURES_H
#define _STRUCTURES_H

#include <event2/event.h>

typedef struct game_node {
    char current_fortune[33];
    char missing_word[33];
    int totalNo;
    int correctNo;
} game_node;

typedef struct node {
    int cfd;        //connection file descriptor
    game_node *status;
    struct event *cev;
    struct node *next;
} tcp_node;

tcp_node* add_node(tcp_node **head, int cfd);

void remove_node(tcp_node **head, int cfd);

#endif
