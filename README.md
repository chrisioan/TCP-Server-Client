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
The purpose of this project is to become familiar with system programming in a Unix environment, specifically focusing on multi-threading and network communication. The goal is to create a program that recursively copies the contents of a directory from a server to the local file system of a client. The server should be capable of handling requests from multiple clients concurrently, processing each request in parallel by breaking it down into independent file copy operations. Similarly, the client must process the data sent by the server and create a local copy of the requested directory, replicating the structure and files exactly as they exist on the server. The fundamental components of the design are the server and the client, and multiple clients can connect simultaneously to a server. Communication between them is facilitated through sockets.
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
1. 
2. 
3. 
<br/><br/>

## Data Server
Source code can be found here: [dataServer.cpp](https://google.com)
<br/><br/>

## Remote Client
Source code can be found here: [remoteClient.cpp](https://google.com)
<br/><br/>