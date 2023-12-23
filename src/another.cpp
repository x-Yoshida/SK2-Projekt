#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <error.h>
#include <sys/epoll.h>
#include <vector>

const int MAX_EVENTS = 10;
const int BUFFER_SIZE = 1024;


void setReuseAddr(int sock){
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno, "setsockopt failed");
}


int main() {
    // Create a UDP socket
    int serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    setReuseAddr(serverSocket);
    // Set up the server address
    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(12345); // Use the desired port number

    // Bind the socket to the server address
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    // Create an epoll instance
    int epollFd = epoll_create1(0);

    // Set up epoll event structure
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET; // Enable edge-triggered mode
    event.data.fd = serverSocket;

    // Add the server socket to the epoll instance
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);

    // Vector to store client sockets
    std::vector<int> clientSockets;

    while (true) {
        // Wait for events
        struct epoll_event events[MAX_EVENTS];
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);

        // Handle events
        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == serverSocket) {
                // New incoming connection
                sockaddr_in clientAddress;
                socklen_t clientAddressLen = sizeof(clientAddress);
                char buffer[BUFFER_SIZE];

                // Receive data from the client
                ssize_t bytesRead = recvfrom(serverSocket, buffer, BUFFER_SIZE, 0,
                                             (struct sockaddr*)&clientAddress, &clientAddressLen);

                // Handle data (for simplicity, just print it)
                std::cout << "Received from " << inet_ntoa(clientAddress.sin_addr) << ": "
                          << buffer << std::endl;

                // Add the client socket to the epoll instance
                int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
                connect(clientSocket, (struct sockaddr*)&clientAddress, clientAddressLen);
                event.data.fd = clientSocket;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);
                clientSockets.push_back(clientSocket);
            } else {
                // Data available on a client socket
                int clientSocket = events[i].data.fd;
                char buffer[BUFFER_SIZE];
                ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);

                // Handle data (for simplicity, just print it)
                std::cout << "Received from client: " << buffer << std::endl;
            }
        }
    }

    // Close sockets and cleanup

    for (int clientSocket : clientSockets) {
        close(clientSocket);
    }

    close(epollFd);
    close(serverSocket);

    return 0;
}
