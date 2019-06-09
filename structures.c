//
//  Solution by: Shalom-David Anifowoshe
//
//


#include <stdlib.h>
#include "structures.h"

tcp_node* add_node(tcp_node **head, int cfd) {
    tcp_node *current_node = NULL;
    game_node *current_game_node = NULL;

    if (*head == NULL)
    {
        *head = (tcp_node*) malloc(sizeof(tcp_node));
        (*head)->next = NULL;
        current_node = *head;
        goto reset;
    }

    else
    {
        current_node = *head;
        while (current_node->next != NULL) {
            current_node = current_node->next;
        }
        current_node->next = (tcp_node*) malloc(sizeof(tcp_node));
        current_node = current_node->next;
        current_node->next = NULL;
    }

reset:
    current_node->cfd = cfd;
    current_node->cev = NULL;
    current_node->status = (game_node*) malloc(sizeof(game_node));
    current_game_node = current_node->status;


    current_game_node->current_fortune[0] = '\0';
    current_game_node->missing_word[0] = '\0';
    current_game_node->totalNo = 0;
    current_game_node->correctNo = 0;


    return current_node;
}

void remove_node(tcp_node **head, int cfd) {
    tcp_node *current_node = *head;
    if (current_node->cfd == cfd)
    {
        *head = (*head)->next;
        event_free(current_node->cev);
        free(current_node);
        return;
    }

    while (current_node->next != NULL) {
        if (current_node->next->cfd == cfd)
        {
            tcp_node *backup_node = current_node->next;
            current_node->next = current_node->next->next;
            event_free(current_node->cev);
            free(backup_node);
            break;
        }

        current_node = current_node->next;
    }
}
