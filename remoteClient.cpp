//  File Name:  remoteClient.cpp
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 7)             /* Check if program is executed correctly */
    {
        printf("Usage: ./remoteClient -i <server_ip> -p <server_port> -d <directory>\n");
        exit(1);
    }

    return 0;
}