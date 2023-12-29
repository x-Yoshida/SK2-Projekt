#include "handler.h"

Client::Client(int fd,int epollfd): _fd(fd), _epollFd(epollfd)
{
    epoll_event ee {EPOLLIN | EPOLLRDHUP,{.ptr=this}};
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _fd, &ee);
}

Client::~Client(){
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, _fd, nullptr);
    shutdown(_fd, SHUT_RDWR);
    close(_fd);
}
int Client::fd() const 
{
    return _fd;
}

void Client::handleEvent(uint32_t events)
{
    if(events & EPOLLIN) {
        char buffer[256];
        ssize_t count = read(_fd, buffer, 256);
        if(count > 0)
            sendToAllBut(_fd, buffer, count);
        else
            events |= EPOLLERR;
    }
    if(events & ~EPOLLIN){
        remove();
    }
}

void Client::timeoutCounterUp()
{
    _timeoutCounter++;
}

int Client::getTimeoutCounter()
{
    return _timeoutCounter;
}

void Client::write(char * buffer, int count)
{
    if(count != ::write(_fd, buffer, count))
        remove();
    
}

void Client::remove() 
{
    printf("removing %d\n", _fd);
    clients.erase(this);
    delete this;
}

Server::Server(int epollfd,uint16_t port): _epollFd(epollfd)
{
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    if(_sock == -1) 
        error(1, errno, "socket failed");

    sockaddr_in serverAddr{.sin_family=AF_INET, .sin_port=htons((short)port), .sin_addr={INADDR_ANY}};
    int res = bind(_sock, (sockaddr*) &serverAddr, sizeof(serverAddr));
    if(res) 
        error(1, errno, "bind failed");
    
    res = listen(_sock, 1);
    if(res) 
        error(1, errno, "listen failed");

}

Server::~Server()
{
    close(_sock);
}

int Server::sock() const
{
    return _sock;
}

void Server::handleEvent(uint32_t events) 
{
    if(events & EPOLLIN)
    {
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);
        
        auto clientFd = accept(_sock, (sockaddr*) &clientAddr, &clientAddrSize);
        if(clientFd == -1) error(1, errno, "accept failed");
        
        printf("new connection from: %s:%hu (fd: %d)\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientFd);
        
        clients.insert(new Client(clientFd,_epollFd));
    }
    if(events & ~EPOLLIN){
        error(0, errno, "Event %x on server socket", events);
        ctrl_c(SIGINT);
    }
}

