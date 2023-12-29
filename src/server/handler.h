#pragma once
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <signal.h>
#include <unordered_set>


class Client;
extern std::unordered_set<Client*> clients;
extern void sendToAllBut(int fd, char * buffer, int count);
extern void ctrl_c(int);

struct Handler {
    virtual ~Handler(){}
    virtual void handleEvent(uint32_t events) = 0;
};

class Client : public Handler
{
    int _fd;
    int _epollFd;
    int _timeoutCounter=0;
    public:
        Client(int fd,int epollfd);
        virtual ~Client();
        int fd() const;
        virtual void handleEvent(uint32_t events) override;
        void timeoutCounterUp();
        int getTimeoutCounter();
        void write(char * buffer, int count);
        void remove();
};

class Server : public Handler {
    int _sock;
    int _epollFd;
    public:
        Server(int epollfd,uint16_t port);
        virtual ~Server();
        int sock() const;
        virtual void handleEvent(uint32_t events) override;
        
};

