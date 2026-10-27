#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/event.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <chrono>
#include "../utils/utils.h"

// Making our thread non-blocking
// When some erro happens in our fd in listen accept recv or send, they might wait long
// and hence block our thread, so we incorporate this flag set to avoid it
static void setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void callAsync()
{

    // Creating an event object
    struct kevent change_event[4], event[4];
    sockaddr_in server_addr, client_addr;

    // Creating a server socket
    int socket_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    setNonBlocking(socket_listen_fd);
    int socket_connection_fd;
    if (socket_listen_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    // Assigning it the address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    int bindResult = ::bind(socket_listen_fd, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    if (bindResult < 0)
    {
        perror("bind");
        exit(1);
    }

    // Making it listen incoming socket connection(client socket)
    listen(socket_listen_fd, 3);

    // Creating a event queue
    int kq = kqueue();

    // Filling the necessary details in the change_event object. It is basically describing filter
    EV_SET(change_event, socket_listen_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

    // Regestring an event on the queue with the necessary details
    int ke = kevent(kq, change_event, 1, NULL, 0, NULL);
    if (ke == -1)
    {
        perror("kevent");
        exit(1);
    }
    auto cronFrequency = std::chrono::seconds(1);
    auto lastCronTime = std::chrono::steady_clock::now();
    // For counting active clients
    int clients = 0;
    for (;;)
    {

        auto now = std::chrono::steady_clock::now();

        //This would we executed every time control reaches here, ie 1 second as we wake up the kevent
        //Hence,the expired keys will be deleted frequently
        if (now - lastCronTime >= cronFrequency)
        {
            deleteExpKeys();
            lastCronTime = now;
        }
        // Defining a timestamp of 1 second and 0 nanoseconds
        struct timespec timeout = {1, 0};

        // Calling the same kqueue but this time not for registering an event
        // but for checking new event in the queue
        // Check for new events, but do not register new events with
        // the kqueue. Hence the 2nd and 3rd arguments are NULL, 0.
        // Only handle 1 new event per iteration in the loop; 5th
        // argument is 1.
        // 6th arg takes timeout interval after which the kevent wakes up
        int new_events = kevent(kq, NULL, 0, event, 1, &timeout);

        // After waking up in given interval, If no events then again loop goes to begining
        // and checks for expiration of keys
        if (new_events == 0)
            continue;

        if (new_events == -1)
        {
            perror("kevent");
            exit(1);
        }

        // As soon as event(s) is detected in the event queue we run this loop
        // When multiple events are pushed simulateously then it gets agregated and hence we loop
        // in the whole new events came
        for (int i = 0; i < new_events; i++)
        {

            // Identifier of event (event file discriptor)
            int event_fd = event[i].ident;

            // 1. When a client disconnects
            //  When the client disconnects an EOF is sent. By closing the file
            //  descriptor the event is automatically removed from the kqueue.
            // flags is a bitmask, meaning multiple flags can be set simultaneously.
            // It checks if any flags are set and if ev_eof is set that means that client disconnected
            // and remove event from that
            // also : & → checks specific bits in a bitmask. && → checks whether two conditions are both true.
            if (event[i].flags & EV_EOF)
            {
                --clients;
                std::cout << "Client disconnected, Active Clients: " << clients << std::endl;
                close(event_fd);
            }

            // 2. When a client Connects
            //  If the new event's file descriptor is the same as the listening
            //  socket's file descriptor, we are sure that a new client wants
            //  to connect to our socket.
            else if (event_fd == socket_listen_fd)
            {
                // Incoming socket connection on the listening socket.
                // Create a new socket for the actual connection to client.
                socklen_t client_len = sizeof(client_addr);
                socket_connection_fd = accept(event_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_len);
                setNonBlocking(socket_connection_fd);
                clients++;
                std::cout << "Client connected, Active Clients: " << clients << std::endl;
                if (socket_connection_fd == -1)
                {
                    perror("Accept socket error");
                    continue;
                }

                // 2.1 Registering event on this new client socket so that,
                // we can get notified when it sends some bytes
                //  Put this new socket connection also as a 'filter' event
                //  to watch in kqueue, so we can now watch for events on this
                //  new socket.
                EV_SET(change_event, socket_connection_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                int readEv = kevent(kq, change_event, 1, NULL, 0, NULL);
                if (readEv < 0)
                {
                    perror("kevent error");
                }
            }

            // 3. When a socket sends us some byte(data)
            // then this event is trigerred
            // After which we read the bytes sent in buffer
            else if (event[i].filter == EVFILT_READ)
            {
                // Read bytes from socket
                char buf[1024];
                ssize_t bytes_read = recv(event_fd, buf, sizeof(buf), 0);
                if (bytes_read > 0)
                {
                    string message = evaluateAndRespond(buf);
                    std::cout << message << std::endl;
                    send(event_fd, message.c_str(), message.length(), 0);
                }
                printf("read %zu bytes\n", bytes_read);
            }
        }
    }
}

// c++ -std=c++17 -o  build/serv2 main.cpp server/async.cpp utils/evals.cpp utils/resp.cpp utils/store.cpp utils/eviction.cpp config/config.h utils/aof.cpp

// Build send functionality in it