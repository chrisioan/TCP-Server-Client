//  File Name:  dataServer.cpp
#include <iostream>
#include <string.h>

#include <sys/wait.h>                          /* sockets */
#include <sys/types.h>                         /* sockets */
#include <sys/socket.h>                        /* sockets */
#include <netinet/in.h> /* internet sockets */ /* for sockaddr_in */
#include <netdb.h>                             /* gethostbyaddr */
#include <unistd.h>                            /* fork */
#include <stdlib.h>                            /* exit */
#include <ctype.h>                             /* toupper */
#include <signal.h>                            /* signal */
#include <arpa/inet.h>                         /* for hton * */

void child_server(int newsock);
void perror_exit(std::string message);
void sigchld_handler(int sig);

// using namespace std;

int main(int argc, char *argv[])
{
    int port, thread_pool_size, queue_size, block_size, sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    struct hostent *rem;

    if (argc != 9) /* Check if program is executed correctly */
    {
        printf("Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n");
        exit(1);
    }

    for (int i = 1; i < 9; i += 2) /* Initialize constants */
    {
        if (strcmp(argv[i], "-p") == 0)
            port = atoi(argv[i + 1]); /* Initialize port */
        if (strcmp(argv[i], "-s") == 0)
            thread_pool_size = atoi(argv[i + 1]); /* Initialize thread_pool_size */
        if (strcmp(argv[i], "-q") == 0)
            queue_size = atoi(argv[i + 1]); /* Initialize queue_size */
        if (strcmp(argv[i], "-b") == 0)
            block_size = atoi(argv[i + 1]); /* Initialize block_size */
    }

    std::cout << "Server's parameters are:\n";
    std::cout << "port: " << port << "\n";
    std::cout << "thread_pool_size: " << thread_pool_size << "\n";
    std::cout << "queue_size: " << queue_size << "\n";
    std::cout << "Block_size: " << block_size << "\n";

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("Socket creation failed!");

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("Socket binding failed!");

    std::cout << "Server was successfully initialized...\n";

    /* Listen for connections */
    if (listen(sock, 5) < 0)
        perror_exit("Server listening failed!");

    std::cout << "Listening for connections to port " << port << "\n";

    while (1)
    {
        /* accept connection */
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
            perror_exit("Server accept failed!");
        /* Find client's address */
        if ((rem = gethostbyaddr((char *)&client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL)
        {
            herror("gethostbyaddr");
            exit(1);
        }
        printf("Accepted connection from %s\n", rem->h_name);
        // printf("Accepted connection\n");
        switch (fork())
        {        /* Create child for serving client */
        case -1: /* Error */
            perror("fork");
            break;
        case 0: /* Child process */
            close(sock);
            child_server(newsock);
            exit(0);
            close(newsock); /* parent closes socket to client */
            /* must be closed before it gets re-assigned */
        }
    }

    std::cout << "Accepted connection from localhost\n";

    return 0;
}

void child_server(int newsock)
{
    char buf[1];
    while (read(newsock, buf, 1) > 0)
    {                    /* Receive 1 char */
        putchar(buf[0]); /* Print received char */
        /* Capitalize character */
        buf[0] = toupper(buf[0]);
        /* Reply */
        if (write(newsock, buf, 1) < 0)
            perror_exit("write");
    }
    printf("Closing connection.\n");
    close(newsock); /* Close socket */
}

/* Wait for all dead child processes */
void sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

void perror_exit(std::string message)
{
    perror(message.c_str());
    exit(EXIT_FAILURE);
}