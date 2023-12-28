#include "server.h"

int main(int argc,char** argv)
{
    if(argc != 2) error(1, 0, "Need 1 arg (port)");
    uint16_t port = readPort(argv[1]);
    
    signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);
    
    int epollFd = epoll_create1(0);
    servHandler = new Server(epollFd,port);
    
    setReuseAddr(servHandler->sock());
    epoll_event ee {EPOLLIN, {.ptr=servHandler}};
    epoll_ctl(epollFd, EPOLL_CTL_ADD, servHandler->sock(), &ee);
    
    while(true){
        int n = epoll_wait(epollFd, &ee, 1, 5000);
        if(n==-1) 
        {
            error(0,errno,"epoll_wait failed");
            ctrl_c(SIGINT);

        }
        if(n==0)
        {
            //https://linux.die.net/man/2/alarm
            //https://linux.die.net/man/2/setitimer
            //Will need to do one of rhose probably
            for(Client* c : clients)
            {
                c->write("Test\n",6);
            }
        }
        else
        {
            //printf("%d\n",ee.events);
            ((Handler*)ee.data.ptr)->handleEvent(ee.events);
        }
    }
}


uint16_t readPort(char * txt)
{
    char * ptr;
    auto port = strtol(txt, &ptr, 10);
    if(*ptr!=0 || port<1 || (port>((1<<16)-1))) error(1,0,"illegal argument %s", txt);
    return port;
}

void setReuseAddr(int sock)
{
    const int one = 1;
    int res = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    if(res) error(1,errno, "setsockopt failed");
}

void ctrl_c(int)
{
    for(Client * client : clients)
        delete client;
    //close(servFd);
    delete servHandler;
    printf("Closing server\n");
    exit(0);
}

void sendToAllBut(int fd, char * buffer, int count)
{
    auto it = clients.begin();
    while(it!=clients.end()){
        Client * client = *it;
        it++;
        if(client->fd()!=fd)
            client->write(buffer, count);
    }
}
