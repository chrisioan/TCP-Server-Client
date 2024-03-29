//  File Name:  remoteClient.cpp
#include "../include/remoteClient.hpp"

void perror_exit(std::string message);

int main(int argc, char *argv[])
{
    std::string server_ip, directory, path = OUT_DIR, dir, filename, msg = "DONE";
    struct stat buf;
    char input, buffer[512];
    unsigned int server_port;
    int sock, filesize, file_count, count;
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

    directory += "\n"; /* To identify the end */
    if (write(sock, directory.c_str(), directory.length()) < 0)
        perror_exit("write failed");

    if (mkdir(OUT_DIR, 0777))
        if (errno != EEXIST)
            perror_exit("mkdir failed");

    while (1)
    {
        filename = "";
        while (read(sock, &input, 1) > 0)
        { /* Read 1) filename from socket */
            if (input == '\n')
                break;
            filename += input;
        }
        std::cout << "Received: " << filename << std::endl;

        size_t pos = 0;
        path = OUT_DIR;
        while ((pos = filename.find("../")) != std::string::npos)
            filename.erase(0, pos + 3);
            
        /* find all directories / subdirectories and create them */
        pos = 0;
        while ((pos = filename.find("/")) != std::string::npos)
        {
            dir = filename.substr(0, pos);
            path += dir + "/";
            if (mkdir(path.c_str(), 0777))
                if (errno != EEXIST)
                    perror_exit("mkdir failed");
            filename.erase(0, pos + 1);
        }
        /* Create the file */
        filename = path + filename;
        if (stat(filename.c_str(), &buf) != -1)
            rmdir(filename.c_str()); /* If file already exists - delete it */
        std::ofstream fout(filename.c_str());

        /* Read 2) metadata from socket */
        /* Take filesize */
        if (read(sock, &filesize, sizeof(filesize)) < 0)
            perror_exit("read failed");
        filesize = ntohl(filesize);

        if (write(sock, msg.c_str(), sizeof(msg)) < 0)
            perror_exit("write failed");

        /* Take file_count */
        if (read(sock, &file_count, sizeof(file_count)) < 0)
            perror_exit("read failed");
        file_count = ntohl(file_count);

        if (write(sock, msg.c_str(), sizeof(msg)) < 0)
            perror_exit("write failed");

        do
        { /* Read 3) file contents from socket */
            memset(buffer, 0, sizeof(buffer));
            if ((count = read(sock, buffer, sizeof(buffer))) < 0)
                perror_exit("read failed");

            filesize -= count;

            for (int i = 0; i < count; i++)
                fout << buffer[i];

        } while (filesize > 0);

        if (write(sock, msg.c_str(), sizeof(msg)) < 0)
            perror_exit("write failed");

        if (file_count == 0) /* It was the last file */
            break;           /* Client finished */
    }

    close(sock);

    return 0;
}

void perror_exit(std::string message)
{
    perror(message.c_str());
    exit(EXIT_FAILURE);
}