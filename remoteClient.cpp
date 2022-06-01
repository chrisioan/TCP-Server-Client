//  File Name:  remoteClient.cpp
#include <iostream>
#include <string>

#include <sys/types.h>                         /* sockets */
#include <sys/socket.h>                        /* sockets */
#include <netinet/in.h> /* internet sockets */ /* for sockaddr_in */
#include <netdb.h>                             /* gethostbyaddr */
#include <unistd.h>                            /* read, write, close */
#include <stdlib.h>                            /* exit */
#include <string.h>                            /* strlen */

void perror_exit(std::string message)
{
    perror(message.c_str());
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    std::string server_ip, directory;
    unsigned int server_port;
    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    if (argc != 7) /* Check if program is executed correctly */
    {
        printf("Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n");
        exit(1);
    }

    for (int i = 1; i < 7; i += 2) /* Initialize constants */
    {
        if (strcmp(argv[i], "-i") == 0)
            server_ip = argv[i + 1]; /* Initialize server_ip */
        if (strcmp(argv[i], "-p") == 0)
            server_port = atoi(argv[i + 1]); /* Initialize server_port */
        if (strcmp(argv[i], "-d") == 0)
            directory = argv[i + 1]; /* Initialize directory */
    }

    std::cout << "Client's parameters are:\n";
    std::cout << "serverIP: " << server_ip << "\n";
    std::cout << "port: " << server_port << "\n";
    std::cout << "directory: " << directory << "\n";

    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("Socket creation failed");

    /* Find server address */
    if ((rem = gethostbyname(server_ip.c_str())) == NULL)
    {
        herror("gethostbyname failed");
        exit(1);
    }

    server.sin_family = AF_INET; /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(server_port); /* Server port */

    /* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect failed");

    std::cout << "Connecting to " << server_ip << " on port " << server_port << "\n";

    if (write(sock, directory.c_str(), directory.length()) < 0)
        perror_exit("write failed");

    return 0;
}