//  File Name:  dataServer.cpp
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 9)             /* Check if program is executed correctly */
    {
        printf("Usage: ./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n");
        exit(1);
    }

    return 0;
}