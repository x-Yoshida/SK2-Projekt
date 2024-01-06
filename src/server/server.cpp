#include "server.h"

pthread_t connt;
bool done = false;
int main(int argc,char** argv)
{
    if(argc != 2) error(1, 0, "Need 1 arg (port)");
    uint16_t port = readPort(argv[1]);
#ifdef WTO
    pthread_create(&connt,NULL,connectionCheck,NULL);
#endif
    signal(SIGINT, ctrl_c);
    signal(SIGPIPE, SIG_IGN);

    int epollFd = epoll_create1(0);
    servHandler = new Server(epollFd,port);
    CmdHandler cmd(epollFd);
    setReuseAddr(servHandler->sock());
    
    epoll_event ee;

    while(true){
        int n = epoll_wait(epollFd, &ee, 1, -1);
        if(n==-1) 
        {
            if(0)
            {
                error(0,errno,"epoll_wait failed");
                ctrl_c(SIGINT);
            }

        }
        if(n==0)
        {
            //connectionCheck();
        }
        else
        {
            if(ee.data.fd==STDIN_FILENO)
            {
                printf("UwU\n");
            }
            
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
    for(Room* r : rooms)
    {
        delete r;
    }
    delete servHandler;
#ifdef WTO
    done=true;
    pthread_join(connt,NULL);
#endif
    printf("Closing server\n");
    exit(0);
}

void sendToAllBut(int fd, std::string msg)
{
    auto it = clients.begin();
    while(it!=clients.end()){
        Client * client = *it;
        it++;
        if(client->fd()!=fd)
            client->write(msg);
    }
}

void* connectionCheck(void* arg)
{
    while(!done)
    {
        sleep(1);
        std::list<Client*> toDelete;
        for(Client* c : clients)
        {
            c->timeoutCounterUp();
            
            printf("%d\n",c->getTimeoutCounter());
            if(c->getTimeoutCounter()>5)
            {
                toDelete.push_back(c);
            }
        }
        for(Client* c : toDelete)
        {
            printf("Bruh\n");
            c->remove();
        }
        

    }
    return NULL;
}
