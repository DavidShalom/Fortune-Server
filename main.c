//
//  Solution by: Shalom-David Anifowoshe
//
//

#include "tcp_functions.h"
#include "structures.h"
#include "server.h"
#include "server.c"

int main(int argc, char **argv)
{
    struct event *tev;
    char *interfaces[] = { "0.0.0.0", "::", NULL };
    int i, tfd;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s port\n", progname);
        exit(EXIT_FAILURE);
    }

    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
       fprintf(stderr, "%s: signal() %s \n", progname, strerror(errno));
       return EXIT_FAILURE;
    }

    (void) daemon(0, 0);

    openlog(progname, LOG_PID, LOG_DAEMON);

    evb = event_base_new();
    if (! evb)
    {
        syslog(LOG_ERR, "creating event base failed");
        return EXIT_FAILURE;
    }

    for (i = 0; interfaces[i]; i++)
    {

        tfd = tcp_listen(interfaces[i], argv[i+3]);

        if (tfd > -1)
        {
          tev = event_new(evb, tfd, EV_READ|EV_PERSIST, callback, NULL);
          event_add(tev, NULL);
        }

    }

    if (event_base_loop(evb, 0) == -1)
    {
        syslog(LOG_ERR, "event loop failed");
        event_base_free(evb);
        return EXIT_FAILURE;
    }

    closelog();

    event_base_free(evb);

    return EXIT_SUCCESS;
}
