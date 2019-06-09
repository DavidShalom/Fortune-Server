//
//  Solution by: Shalom-David Anifowoshe
//
//


#include "server.h"
#include "tcp_functions.h"
#include "structures.h"

static const char *progname = "server";
static tcp_node *connections = NULL;
static struct event_base *evb;

static void server_to_client(tcp_node *tcp_connection, reply_type rt, char* generic_msg)
{
    char message[200];

    switch (rt) {
        case CHALLENGE:
            sprintf(message, "C: %s", tcp_connection->status->current_fortune);
        break;
        case OK:
            strcpy(message, "O: Congratulations - challenge passed!\n");
        break;
        case FAIL:
            sprintf(message, "F: Wrong guess - expected: %s\n", tcp_connection->status->missing_word);
        break;
        case GENERIC:
            sprintf(message, "M: %s", generic_msg);
        break;
        case UNKNOWN:
        default:
            strcpy(message, "M: unrecognized input from client \n");
    }

    tcp_write(tcp_connection->cfd, message, strlen(message));
}

static void callback(evutil_socket_t evfd, short evwhat, void *evarg) {
    size_t n; int cfd;
    char message[200];
    tcp_node *new_connection;


    cfd = tcp_accept((int)evfd);    //accepts and opens
    if (cfd == -1)
    {
        return;
    }

    strcpy(message, "M: Guess the missing ____!\n");

    strcat(message, "M: Send your guess in the form ’R: word\\r\\n’.\n");

    n = tcp_write(cfd, message, strlen(message));
    if (n != strlen(message)) {
        syslog(LOG_ERR, "write failed");
        tcp_close(cfd);
    }

    new_connection = add_node(&connections, cfd);  //add cfd to linked list

    //here, input is read from the client
    new_connection->cev = event_new(evb, cfd, EV_READ|EV_PERSIST, read_client_message, new_connection);

    event_add(new_connection->cev, NULL);

    get_fortune(new_connection);
}

static void get_fortune(tcp_node *tcp_connection)
{
    pid_t child_process;
    int pipe_fd[2];
    struct event *fev;


    pipe(pipe_fd);

    fev = event_new(evb, pipe_fd[0], EV_READ, read_fortune, tcp_connection);

    event_add(fev, NULL);

    child_process = fork();

    if (child_process == -1)
    {
        syslog(LOG_ERR, "failed to fork process");
        tcp_connection->status->current_fortune[0] = '\0';
    }

    else if (child_process == 0) // redirect to the pipe
     {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        execlp("fortune", "fortune", "-n", "32", "-s", NULL);
        syslog(LOG_ERR, "failed to start fortune");
        exit(EXIT_FAILURE);
    }

    else
    {
        close(pipe_fd[1]);
    }
}
static void read_fortune(evutil_socket_t evfd, short evwhat, void *evarg) {
    int nbytes, pipe_fd = evfd;
    tcp_node *present = (tcp_node*) evarg;

    nbytes = read(pipe_fd, present->status->current_fortune, 32); //read fortune pipe_fd and store in the present->status->current_fortune
    if (nbytes == -1)
    {
        syslog(LOG_ERR, "failed to read from fortune");
        present->status->current_fortune[0] = '\0';
    }
    else
    {
        present->status->current_fortune[nbytes] = '\0';
    }

    //replaces word
    replace_word(present->status);
    //sends after the word is replaced
    server_to_client(present, CHALLENGE, NULL);
}

static void read_client_message(evutil_socket_t evfd, short evwhat, void *evarg) {
    tcp_node *present = (tcp_node*) evarg;
    int nbytes, cfd = evfd;
    char message[200], *token;
    char delimit[] = " \t\r\n\v\f";

    nbytes = read(cfd, message, sizeof(message));     //we read from the client here(netcat used to create the cient)
    if (nbytes < 0) {
        syslog(LOG_ERR, "error reading from client");
        tcp_close(cfd); remove_node(&connections, cfd);
        return;
    }

    if (strstr(message, "Q:") == message) //checks the kind of message it is
    {
        sprintf(message, "You mastered %d/%d challenges. Good bye!\n",
                            present->status->correctNo,
                            present->status->totalNo);

        server_to_client(present, GENERIC, message);

        tcp_close(cfd); remove_node(&connections, cfd);
    }
    else
    {
        if (strstr(message, "R:") == message) //checks again
        {
            present->status->totalNo++;

            memmove(message, message + 2, strlen(message) - 2);

            token = strtok(message, delimit);

            if (token == NULL || strcmp(token, present->status->missing_word) != 0)
            {
                server_to_client(present, FAIL, NULL);
            }

            else
            {
                present->status->correctNo++;
                server_to_client(present, OK, NULL);
            }

            get_fortune(present); //new challenge should be sent here
        }

        else
        {
            server_to_client(present, GENERIC, "Command unrecognized. Please try again!\n");
        }
    }
}

static void replace_word(game_node* status) {
    int count, word_number;
    char *token, *position;
    char fortune[33];
    char delimit[] = " \t\r\n\v\f.,;!~`_-";

    //count tokens
    position = status->current_fortune;
    while ((position = strpbrk(position, delimit)) != NULL) {
        position++; count++;
    }

replace:
    // randomly choose word
    srand(time(NULL));
    word_number = rand() % count;
    count = 0;

    // go through phrase word by word
    strcpy(fortune, status->current_fortune);
    token = strtok(fortune, delimit);
    while (token) {
        if (count == word_number)  //remove word
        {
            position = strstr(status->current_fortune, token);
            if (!position) //try again if there is an error
            {
                goto replace;
            }

            memset(position, '_', strlen(token));
            strcpy(status->missing_word, token);
            break;
        }
        count++;
        token = strtok(NULL, delimit);
    }
    if (token == NULL) //In the case that no word is chosen
    {
        goto replace;
    }
}
