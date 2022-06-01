//  File Name:  dataServer.cpp
#include <iostream>
#include <string.h>
#include <queue>

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
#include <pthread.h>                           /* for threads */
#include <dirent.h>                            /* for readdir */

int newsock;
std::queue<std::string> work_queue;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar; /* Condition variable */

struct thread_args
{ /* struct for threads arguments */
    int sock;
    unsigned int queue_size;
};

void *comm_thread(void *argp);
void *worker_thread(void *argp);
void perror_exit(std::string message);
void _handler(int signum);

int main(int argc, char *argv[])
{
    static struct sigaction act;
    act.sa_handler = _handler;
    act.sa_flags = SA_RESTART;
    sigfillset(&(act.sa_mask));
    sigaction(SIGINT, &act, NULL);

    unsigned int port, thread_pool_size, queue_size, block_size;
    int sock;
    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof(client);
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    struct hostent *rem;

    if (argc != 9) /* Check if program is executed correctly */
    {
        printf("Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n");
        exit(1);
    }

    for (int i = 1; i < 9; i += 2) /* Initialize arguments */
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
        perror_exit("Socket creation failed");

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("Socket binding failed");

    std::cout << "Server was successfully initialized...\n";

    /* Listen for connections */
    if (listen(sock, 5) < 0)
        perror_exit("Server listening failed");

    std::cout << "Listening for connections to port " << port << "\n";

    while (1)
    {
        /* accept connection */
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
            perror_exit("Server accept failed");

        /* Find client's address */
        if ((rem = gethostbyaddr((char *)&client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL)
        {
            herror("gethostbyaddr failed");
            exit(1);
        }
        printf("Accepted connection from %s\n\n", rem->h_name);

        thread_args ta;
        ta.sock = newsock;
        ta.queue_size = queue_size;

        pthread_cond_init(&cvar, NULL); /* Initialize condition variable */

        pthread_t thr;
        int err;
        /* New Communication Thread */
        if ((err = pthread_create(&thr, NULL, comm_thread, (void *)&ta)))
            perror_exit("pthread_create failed");
    }

    return 0;
}

void *comm_thread(void *argp)
{
    int err;
    char buf;
    std::string path, directory = "";
    thread_args ta = *(thread_args *)argp;
    DIR *dir;
    struct dirent *dir_entry;

    while (read(ta.sock, &buf, 1) > 0)
        directory += buf; /* Read from client the directory to be copied */

    std::cout << "[Thread: " << pthread_self() << "]: About to stan directory " << directory << "\n";

    if ((dir = opendir(directory.c_str())) == NULL)
        perror_exit("opendir failed");

    while ((dir_entry = readdir(dir)) != NULL)
    {
        path = directory + "/";
        if (dir_entry->d_name == NULL || strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
            continue;

        path += dir_entry->d_name;
        if (dir_entry->d_type == DT_REG)
        {
            if (work_queue.size() == ta.queue_size) /* Queue is full - can't add more */
                pthread_cond_wait(&cvar, &mtx);     /* Wait for signal */

            /* Lock mutex */
            if ((err = pthread_mutex_lock(&mtx)))
                perror_exit("pthread_mutex_lock failed");

            std::cout << "[Thread: " << pthread_self() << "]: Adding file " << path << " to the queue...\n";
            work_queue.push(path);

            /* Unlock mutex */
            if ((err = pthread_mutex_unlock(&mtx)))
                perror_exit("pthread_mutex_unlock");

            if (work_queue.size() - 1 == 0) /* Queue not empty anymore */
                pthread_cond_signal(&cvar); /* Awake other thread */
        }
        else if (dir_entry->d_type == DT_DIR)
        {
        }
    }
    closedir(dir);

    // std::string out;
    // while (!work_queue.empty())
    // {
    //     out = work_queue.front();
    //     work_queue.pop();
    //     std::cout << out << "\n";
    // }

    pthread_exit(NULL);
}

void *worker_thread(void *argp)
{
    thread_args ta = *(thread_args *)argp;

    if (work_queue.empty())             /* Queue is empty */
        pthread_cond_wait(&cvar, &mtx); /* Wait for signal */

    if (work_queue.size() + 1 == ta.queue_size) /* Queue not full anymore */
        pthread_cond_signal(&cvar);             /* Awake other thread */

    pthread_exit(NULL);
}

void perror_exit(std::string message)
{
    perror(message.c_str());
    exit(EXIT_FAILURE);
}

void _handler(int signum)
{
    if (signum == SIGINT)
    {
        close(newsock); // only the current (?)
        raise(SIGKILL);
    }
}