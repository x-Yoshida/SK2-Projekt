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
//#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <unordered_set>
#include <vector>

#include "room.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

std::vector<std::string> splitBy(char* line,char sep=' ',char end='\n');
char* itoa(int val,char* str);

class Room;
class Client;
extern std::unordered_set<Client*> clients;
extern std::unordered_set<Room*> rooms;

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
    std::string _name;
    public:
        Client(int fd,int epollfd);
        virtual ~Client();
        int fd() const;
        std::string name() const;
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

class CmdHandler : public Handler
{
    public:
        CmdHandler(int epollFd);
        virtual void handleEvent(uint32_t events) override;
        void sendTo(int fd, char * buffer, int count);
        void listConnected();
        void listRooms();
};



