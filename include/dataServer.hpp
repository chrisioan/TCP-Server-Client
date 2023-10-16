//  File Name:  dataServer.hpp
#include <iostream>
#include <string.h>
#include <queue>
#include <vector>
#include <map>
#include <fstream>
#include <fcntl.h>
#include <sys/wait.h>   /* sockets */
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <netdb.h>      /* gethostbyaddr */
#include <unistd.h>
#include <stdlib.h>    /* exit */
#include <signal.h>    /* signal */
#include <arpa/inet.h> /* for hton */
#include <pthread.h>   /* for threads */
#include <dirent.h>    /* for readdir */