//  File Name:  dataServer.cpp
#include <iostream>
#include <string.h>
#include <queue>
#include <vector>
#include <map>
#include <iterator>
#include <fstream>
#include <fcntl.h>

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

int server_sock;
int newsock;
pthread_t *tids;
std::queue<std::map<std::string, int>> work_queue; /* Stores pair <file, socket> */
std::map<int, int> files_per_socket;               /* Stores pair <socket, num_of_files> */
std::map<int, pthread_mutex_t> sock_mtx;           /* Stores pair <socket, mutex> */
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;   /* Mutex for work_queue */
pthread_cond_t cvar1, cvar2;                       /* Condition variable for work_queue*/

struct thread_args
{ /* struct for threads arguments */
    int sock;
    unsigned int queue_size, block_size;
    pthread_mutex_t mtx; /* Mutex for not writing in same socket */
};

void find_files(struct dirent *dir_entry, thread_args ta, pthread_t thr_id, std::string path, int flag);
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
    struct sockaddr_in server, client;
    socklen_t clientlen = sizeof(client);
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    struct hostent *rem;
    thread_args ta;

    if (argc != 9) /* Check if program is executed correctly */
    {
        std::cout << "Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n";
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

    pthread_cond_init(&cvar1, NULL); /* Initialize condition variable */
    pthread_cond_init(&cvar2, NULL); /* Initialize condition variable */

    ta.queue_size = queue_size;
    ta.block_size = block_size;
    tids = new pthread_t[thread_pool_size];
    /* Create <thread_pool_size> Worker Thread(s) */
    for (unsigned int i = 0; i < thread_pool_size; i++)
        if (pthread_create(&tids[i], NULL, worker_thread, (void *)&ta))
            perror_exit("pthread_create failed");

    /* Create socket */
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("Socket creation failed");

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    /* Bind socket to address */
    if (bind(server_sock, serverptr, sizeof(server)) < 0)
        perror_exit("Socket binding failed");

    std::cout << "Server was successfully initialized...\n";

    /* Listen for connections */
    if (listen(server_sock, 1000) < 0)
        perror_exit("Server listening failed");

    std::cout << "Listening for connections to port " << port << "\n";

    while (1)
    {
        /* accept connection */
        if ((newsock = accept(server_sock, clientptr, &clientlen)) < 0)
            perror_exit("Server accept failed");

        /* Find client's address */
        if ((rem = gethostbyaddr((char *)&client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL)
        {
            herror("gethostbyaddr failed");
            exit(1);
        }
        std::cout << "Accepted connection from " << rem->h_name << "\n\n";

        pthread_t thr;
        ta.sock = newsock;
        ta.mtx = PTHREAD_MUTEX_INITIALIZER;
        /* New Communication Thread */
        if (pthread_create(&thr, NULL, comm_thread, (void *)&ta))
            perror_exit("pthread_create failed");
    }

    return 0;
}

void find_files(struct dirent *dir_entry, thread_args ta, pthread_t thr_id, std::string path, int flag)
{
    DIR *dir;
    path += dir_entry->d_name;
    std::map<std::string, int> entry;

    if (dir_entry->d_name == NULL || strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
        return;

    if (dir_entry->d_type == DT_DIR) /* if it's a directory */
    {
        if ((dir = opendir(path.c_str())) == NULL)
            perror_exit("opendir failed");

        path += "/";

        if (flag == 0) /* Find total amount of files */
        {
            while ((dir_entry = readdir(dir)) != NULL)
                find_files(dir_entry, ta, thr_id, path, 0); /* Recursion */
        }
        else
        {
            while ((dir_entry = readdir(dir)) != NULL)
                find_files(dir_entry, ta, thr_id, path, 1); /* Recursion */
        }
        closedir(dir);
    }
    else if (dir_entry->d_type == DT_REG) /* if it's a regular file */
    {
        if (flag == 0)
            files_per_socket.find(ta.sock)->second += 1; /* Increase counter by 1 */
        else
        {
            /* Lock mutex */
            if (pthread_mutex_lock(&mtx))
                perror_exit("pthread_mutex_lock failed");

            if (work_queue.size() == ta.queue_size) /* Queue is full - can't add more */
                pthread_cond_wait(&cvar1, &mtx);    /* Wait for signal */

            std::cout << "[Thread: " << thr_id << "]: Adding file " << path << " to the queue...\n";
            entry.insert(std::pair<std::string, int>(path, ta.sock)); /* Create the pair <file, socket> */
            work_queue.push(entry);                                   /* Add the pair to the Queue */

            if (work_queue.size() > 0)       /* Queue not empty */
                pthread_cond_signal(&cvar2); /* Awake other thread */

            /* Unlock mutex */
            if (pthread_mutex_unlock(&mtx))
                perror_exit("pthread_mutex_unlock");
        }
    }
}

void *comm_thread(void *argp)
{
    char buf;
    std::string path, directory = "";
    thread_args ta = *(thread_args *)argp;
    DIR *dir, *dir2;
    struct dirent *dir_entry;

    while (read(ta.sock, &buf, 1) > 0)
    { /* Read from client's socket the directory to be copied */
        if (buf == '\n')
            break;
        directory += buf;
    }

    std::cout << "[Thread: " << pthread_self() << "]: About to scan directory " << directory << "\n";

    files_per_socket.insert(std::pair<int, int>(ta.sock, 0));          /* Initialize socket's number of files */
    sock_mtx.insert(std::pair<int, pthread_mutex_t>(ta.sock, ta.mtx)); /* Initialize socket's mutex */

    if ((dir = opendir(directory.c_str())) == NULL)
        perror_exit("opendir failed");

    path += directory + "/";

    while ((dir_entry = readdir(dir)) != NULL)              /* Find total amount of files */
        find_files(dir_entry, ta, pthread_self(), path, 0); /* Including those in subfolders - using recursion */
    closedir(dir);

    if ((dir2 = opendir(directory.c_str())) == NULL)
        perror_exit("opendir failed");

    while ((dir_entry = readdir(dir2)) != NULL)             /* Add all the files 'directory' dir to Queue */
        find_files(dir_entry, ta, pthread_self(), path, 1); /* Including those in subfolders - using recursion */
    closedir(dir2);

    pthread_exit(NULL);
}

void *worker_thread(void *argp)
{
    std::string filename;
    thread_args ta = *(thread_args *)argp;
    char buffer[ta.block_size];
    uint32_t in;
    int filesize, fd, count;
    std::map<std::string, int> entry;
    std::ifstream file;
    pthread_mutex_t *m; /* Need to have a pointer to a mutex - will find it from 'sock_mtx map' */

    while (1)
    {
        /* Lock mutex */
        if (pthread_mutex_lock(&mtx))
            perror_exit("pthread_mutex_lock failed");

        if (work_queue.empty())              /* Queue is empty */
            pthread_cond_wait(&cvar2, &mtx); /* Wait for signal */

        entry = work_queue.front();          /* Take the first pair <file, socket> */
        work_queue.pop();                    /* Remove it from Queue */
        filename = entry.begin()->first;     /* Extract <file> */
        ta.sock = entry.begin()->second;     /* Extract <socket> */
        m = &sock_mtx.find(ta.sock)->second; /* Find socket's mutex */
        std::cout << "[Thread: " << pthread_self() << "]: Received task: <" << filename << ", " << ta.sock << ">\n";

        if (work_queue.size() < ta.queue_size) /* Queue not full */
            pthread_cond_signal(&cvar1);       /* Awake other thread */

        /* Unlock mutex */
        if (pthread_mutex_unlock(&mtx))
            perror_exit("pthread_mutex_unlock");

        FILE *file; /* So that we can use fseek() and ftell() to get file_size*/
        if ((file = fopen(filename.c_str(), "r")) == NULL)
            perror_exit("fopen failed");

        fseek(file, 0L, SEEK_END); /* Go to the end of the file */
        filesize = ftell(file);    /* Store how many bytes there are */
        fclose(file);

        if ((fd = open(filename.c_str(), O_RDONLY)) < 0)
            perror_exit("open failed");

        /* Lock mutex */
        if (pthread_mutex_lock(m))
            perror_exit("pthread_mutex_lock failed");

        files_per_socket.find(ta.sock)->second -= 1; /* Decrease counter by 1 */

        filename += "\n";
        /* Write 1) filename to socket */
        if (write(ta.sock, filename.c_str(), filename.length()) < 0)
            perror_exit("write failed");

        /* Write 2) metadata to socket */
        in = htonl(filesize);
        if (write(ta.sock, &in, sizeof(in)) < 0) /* Write filesize */
            perror_exit("write failed");

        memset(buffer, 0, sizeof(buffer)); /* Clear buffer */
        while (read(ta.sock, buffer, sizeof(buffer)) > 0)
            if (strcmp(buffer, "DONE") == 0) /* Wait for response that it got previous write */
                break;

        in = htonl(files_per_socket.find(ta.sock)->second);
        if (write(ta.sock, &in, sizeof(in)) < 0) /* Write num_of_files left */
            perror_exit("write failed");

        memset(buffer, 0, sizeof(buffer)); /* Clear buffer */
        while (read(ta.sock, buffer, sizeof(buffer)) > 0)
            if (strcmp(buffer, "DONE") == 0) /* Wait for response that it got previous write */
                break;

        memset(buffer, 0, sizeof(buffer)); /* Clear buffer */

        while ((count = read(fd, buffer, sizeof(buffer))) > 0) /* Read a block from the file */
        {                                                      /* Write 3) file contents to socket */
            if (write(ta.sock, &buffer, count) < 0)
                perror_exit("write failed");
            memset(buffer, 0, sizeof(buffer)); /* Clear buffer */
        }

        memset(buffer, 0, sizeof(buffer)); /* Clear buffer */
        while (read(ta.sock, buffer, sizeof(buffer)) > 0)
            if (strcmp(buffer, "DONE") == 0) /* Wait for response that it got previous write */
                break;

        /* Unlock mutex */
        if (pthread_mutex_unlock(m))
            perror_exit("pthread_mutex_unlock");

        if (files_per_socket.find(ta.sock)->second == 0)
        {
            close(ta.sock);
            pthread_mutex_destroy(m);
        }

        close(fd);
    }

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
        close(server_sock); /* Close server's socket */
        close(newsock);

        pthread_mutex_destroy(&mtx);

        delete[] tids;

        raise(SIGKILL);
    }
}