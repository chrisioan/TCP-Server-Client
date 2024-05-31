# TCP Server/Client

## Table of contents
* [About The Project](#about-the-project)
* [Requirements](#requirements)
* [How To Run](#how-to-run)
* [General Notes](#general-notes)
* [Data Server](#data-server)
* [Remote Client](#remote-client)
<br/><br/>

## About The Project
The purpose of this project is to become familiar with system programming in a Unix environment, specifically focusing on multi-threading and network communication. The goal is to create a program that recursively copies the contents of a directory from a server to the local file system of a client.

The server should be capable of handling requests from multiple clients concurrently, processing each request in parallel by breaking it down into independent file copy operations. Similarly, the client must process the data sent by the server and create a local copy of the requested directory, replicating the structure and files exactly as they exist on the server.

The fundamental components of the design are the server and the client, and multiple clients can connect simultaneously to a server. Communication between them is facilitated through sockets.
<br/><br/>

## Requirements
* Make
  ```sh
  sudo apt install make
  ```
* Compiler with support for C++11 or newer
  ```sh
  sudo apt install g++
  ```
<br/><br/>

## How To Run 
When on the root directory of the project:
```sh
make
```
Then navigate to **build/release** and
* For **dataServer**, execute:
```sh
./dataServer -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>
```
Where:
1. <u>port</u>: The port on which the server will listen for external connections.
2. <u>thread_pool_size</u>: The number of worker threads in thread pool.
3. <u>queue_size</u>: The number of positions in the execution queue.
4. <u>block_size</u>: The size of the file blocks in bytes that the worker threads will send.

* For **remoteClient**, execute:
```sh
./remoteClient -i <server_ip> -p <server_port> -d <directory>
```
Where:
1. <u>server_ip</u>: The IP address used by the server.
2. <u>server_port</u>: The port on which the server listens for external connections.
3. <u>directory</u>: The directory to be copied (a relative path).
---
* Example:
```sh
./dataServer -p 12500 -s 2 -q 2 -b 512
```
```sh
./remoteClient -i 127.0.0.1 -p 12500 -d Server
```
<br/><br/>

## General Notes
1. There are parts of code that I have taken from the class slides (function "perror_exit," code for creating the sockets, including the structs, initializing their fields, bind, listen, accept, etc.).
2. When providing the argument `<directory>` for ./remoteClient, please do NOT include a `slash (/)` at the end of the argument. In some cases (e.g. when there are subdirectories), it is added already, so if you include a slash, it could result in a path like `Server//test/file1` which may cause issues.
3. The creation of the directory to copy `(<directory>)`, is done within another folder called `output (build/release/output)`, which is defined in [remoteClient.hpp](https://github.com/chrisioan/TCP-Server-Client/blob/main/include/remoteClient.hpp).
<br/><br/>

## Data Server
Source code can be found here: [dataServer.cpp](https://github.com/chrisioan/TCP-Server-Client/blob/main/src/dataServer.cpp)

While it's not mandatory, I have created a sigaction to detect the SIGINT signal in order to perform partial cleanup.

Initially, there is a check to verify if the program has been executed correctly. After that, the provided arguments are initialized, as well as the conditional variables and mutex that will be needed for accessing the execution queue (work_queue).

Then, the program creates the thread pool (tids), which generates a specified number of worker threads equal to <thread_pool_size>. In an infinite while loop (while(1)), the dataServer continually accepts incoming connections and creates a new communication thread, passing the socket received from the accept as an argument.

In the communication thread (comm_thread), a mutex is initialized, which will be used to lock each worker thread and prevent access for reading/writing on the same socket by another worker thread. The server needs to ensure that, during its execution, only one worker thread writes data to a client's socket at a time.

This mutex is declared as "static" to prevent it from being destroyed when the communication thread ends (it will be destroyed by a worker thread, as described later). It is declared as "thread_local" to ensure that each different communication thread creates its own mutex, making it unique each time a new thread is created.

At this point, it's worth mentioning that the execution queue contains a map with filenames as keys and their corresponding sockets as values. Additionally, two other auxiliary maps have been implemented to provide necessary information to the worker threads:
1. "files_per_socket" with sockets as keys and the number of files associated with each socket as values.
2. "sock_mtx" with sockets as keys and mutex pointers as values.

In the communication thread, the following actions take place:
1. The thread reads the directory name to be copied from the socket.
2. It initializes the value of the socket in the "files_per_socket" map to 0 and sets the "sock_mtx" map entry with the previously created mutex.
3. The recursive function "find_files" is called twice, once with a flag of 0 (count the number of files) and once with a flag of 1 (enqueue the files for execution).

In the "find_files" function, the following actions are performed:
1. It checks if the entry is a directory. If it is, the "find_files" function is recursively called with the updated path and dir_entry.
2. If the entry is a regular file, it should be added to the execution queue. To achieve this, a global mutex is locked, the necessary work is performed, and then the mutex is unlocked.
3. After locking the mutex, there is a check to see if the execution queue is full. If it's full, the first condition variable should wait until there is space available. If it's okay to add the file, the appropriate message is printed, and the file (with its associated socket) is added to the queue.
4. After this step, if the queue is not empty, the second condition variable (for the worker thread) is signaled.

In the communication thread, the following actions occur within an infinite while loop (while(1)):
1. Lock the mutex.
2. If the execution queue is empty, wait on the second condition variable until a signal is received from the communication thread calling the recursive function "find_files" ("In case the execution queue is empty, worker threads should wait until a record is available").
3. If it's okay to proceed, acquire the first file (with its associated socket) from the queue and its corresponding mutex from the "sock_mtx" map. Print the appropriate message.
4. Check if the work_queue is no longer full to signal the first condition variable (allowing a communication thread to write) and unlock the global mutex.
5. Open the file and use fseek with appropriate arguments to determine the number of bytes in the file. This information will be sent as metadata to inform the remoteClient when it has finished reading a file.
6. Lock the mutex previously acquired to prevent other worker threads from writing to the same socket. Decrease the file count in the "files_per_socket" map and store the updated value in a variable called "file_count."
7. First, write the filename (which also contains the path, to be split into pieces on the remoteClient).
8. Then, write the metadata, including the filesize and the file_count. File_count is necessary for the remoteClient to know when to stop, indicating that it has received all the requested files.
9. In a while loop, read the file's contents and write them over the socket in blocks (up to <block_size> bytes per block).
10. In between, there are reads and message exchanges between the worker thread and the remoteClient to ensure proper communication.
11. Finally, unlock the mutex and check if "file_count" is zero. If it is, the socket's data must be removed from the two maps, the one end of the socket should be closed, and the mutex should be destroyed.
<br/><br/>

## Remote Client
Source code can be found here: [remoteClient.cpp](https://github.com/chrisioan/TCP-Server-Client/blob/main/src/remoteClient.cpp)

The program's execution begins with the initialization of arguments. It creates a socket, connects to the dataServer, and starts writing the directory to copy. In an infinite while loop (while(1)), it first reads the filename (including the path). It then breaks down the path into segments, creating all the necessary subdirectories. Subsequently, it creates the file (deletes it if it already exists), reads the metadata (sending "OK" messages after receiving them), and then reads the file's contents until the filesize becomes zero.

There is no information on the [paper](https://github.com/chrisioan/TCP-Server-Client/blob/main/hw2-spring-2022_paper.pdf) regarding reading the file's contents from the socket - whether that needs to be done block by block so my approach is to use a fixed buffer size of 512. If the assignment required reading in blocks, one solution could be to include the block size as metadata and adjust the buffer size accordingly after the remoteClient receives this metadata.

Finally, if "file_count" is set to 0, the remoteClient terminates.
<br/><br/>
