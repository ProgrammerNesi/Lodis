#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include "../utils/utils.h"
#include<unistd.h>
void callSocket()
{
    // 1 .Creating a socket

    // socket() creates an endpoint for communication and returns a file
    // descriptor that refers to that endpoint. And if erro then -1 is returned
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1)
    {
        // std::perror is a built-in function that prints
        // a descriptive error message directly to the standard error stream (stderr).
        // It translates the system error code currently stored in the global variable errno
        // into a human-readable format
        perror("socket");
    }

    // 2 . Creating address of socket

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // 3 . Assigning address to socket

    // bind() assigns the address specified by addr to the socket referred to by
    // the file descriptor sockfd
    int bindResult = ::bind(sockFd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult == -1)
    {
        perror("bind");
    }

    // Infinite loop to keep the server running and accepting connections
    while (true)
    {

        // 4 . Listening for incoming connections in server socket

        // listen() marks the socket referred to by sockfd as a passive socket,
        // that is, as a socket that will be used to accept incoming connection requests
        // second argument specifies the maximum length for the queue of pending connections.
        int listenResult = listen(sockFd, SOMAXCONN);
        if (listenResult == -1)
        {
            perror("listen");
        }

        // 5 . Accepting a connection from a client

        // accept() extracts the first connection request on the queue of pending connections
        // It extracts the first connection request on the queue of pending connections for the
        // listening socket, sockfd, creates a new connected socket, and
        // returns a new file descriptor referring to that socket
        int clientSocket = accept(sockFd, nullptr, nullptr);
        if (clientSocket == -1)
        {
            perror("accept");
        }


        // 6 . Receiving data from the client

        // recv() is used to receive messages from a socket
        // The only difference between recv() and read() is the presence of flags.
        // If no flags are set, recv() is equivalent to read()
        while (true)
        {
            char buffer[1024] = {0};
            int bytesRead=recv(clientSocket, buffer, sizeof(buffer), 0);
            if(bytesRead <= 0){
                close(clientSocket);
                cout << "Client disconnected"<<endl;
                break;
            }
            cout << "Message from client: " << buffer << endl;
            string message = evaluateAndRespond(buffer);
            send(clientSocket, message.c_str(), message.length(), 0);

        }
    }
}
//command to run  c++ -std=c++17 -o server Lodis/server/sync.cpp Lodis/utils/utils.cpp Lodis/utils/evals.cpp Lodis/utils/resp.cpp -lpthread

// c++ -std=c++17 -o build/serv server/sync.cpp utils/evals.cpp utils/resp.cpp

//c++ -std=c++17 -o  build/serv main.cpp server/sync.cpp utils/evals.cpp utils/resp.cpp

