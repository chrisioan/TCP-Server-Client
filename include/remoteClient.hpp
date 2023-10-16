//  File Name:  remoteClient.hpp
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <fstream>
#include <sys/types.h>                         /* sockets */
#include <sys/socket.h>                        /* sockets */
#include <netinet/in.h>                        /* internet sockets */
#include <netdb.h>                             /* gethostbyaddr */
#include <unistd.h>                            /* read, write, close */
#include <stdlib.h>                            /* exit */
#include <string.h>                            /* strlen */

#define OUT_DIR "output/"