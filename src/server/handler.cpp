#include "handler.h"

std::vector<std::string> splitBy(char* line,char sep,char end)
{
    std::vector<std::string> res;
    //int i=0;
    while(*line!='\0')
    {
        if(*line==sep || *line==end)
        {
            line++;
            continue;
        }
        std::string tmp="";
        while(*line!=sep && *line!=end && *line!='\0')
        {
            tmp+=*line;
            line++;
        }
        res.push_back(tmp);
    }
    return res;
}

char* itoa(int val,char* str)
{
    std::stringstream ss;
    ss << val;
    strcpy(str,ss.str().c_str());
    return str;
}

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

std::string Client::name() const
{
    return _name;
}

void Client::handleEvent(uint32_t events)
{
    if(events & EPOLLIN) {
        char buffer[BUFFER_SIZE];
        memset(buffer,0,BUFFER_SIZE);
        ssize_t count = read(_fd, buffer, BUFFER_SIZE);
        if(count > 0)
        {
            std::vector<std::string> msgv = splitBy(buffer);
            printf("%d: %s",_fd,buffer);
            
            printf("%s\n",msgv[0].c_str());

            if(!strcmp(msgv[0].c_str(),"SETNICK"))
            {
                _name=msgv[1];
                write("NICKACCEPTED",13);
            }
            if(!strcmp(msgv[0].c_str(),"ROOMS"))
            {
                //To be changed
                std::stringstream ss;
                ss <<"Rooms\n";
                for(Room* r : rooms)
                {
                    ss<< "ROOM|"<<r->name()<<"|("<<r->currentPlayers()<<"/"<<r->maxPlayers()<<")";
                }
                std::string tmp = ss.str();
                write((char*)tmp.c_str(),tmp.length());
            }

            if(!strcmp(msgv[0].c_str(),"JOIN"))
            {
                for(Room* r : rooms)
                {
                    if(r->name()==msgv[1])
                    {
                        r->join(this);
                    }
                }
            }
            //sendToAllBut(_fd, buffer, count);
        }
        else
        {
            events |= EPOLLERR;
        }
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
    epoll_event ee {EPOLLIN, {.ptr=this}};
    epoll_ctl(_epollFd, EPOLL_CTL_ADD, _sock, &ee);

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

void CmdHandler::handleEvent(uint32_t events)
{
    if(events & EPOLLIN)
    {
        char buffer[BUFFER_SIZE]={};
        ssize_t count = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        
        std::vector<std::string> cmd = splitBy(buffer);
        if(!strcmp(cmd[0].c_str(),"lc"))
        {
            listConnected();
        }
        if(!strcmp(cmd[0].c_str(),"lr"))
        {
            listRooms();
        }
        if(!strcmp(cmd[0].c_str(),"cr"))
        {
            rooms.insert(new Room(cmd[1]));
        }
        
        if(!strcmp(cmd[0].c_str(),"st"))
        {
            std::string msg = "";
            for(int i=2;i<cmd.size()-1;i++)
            {
                msg += cmd[i] + " ";
            }
            msg+=cmd[cmd.size()-1]+"\n";
            sendTo(atoi(cmd[1].c_str()),(char *)(msg.c_str()),msg.length());
        }
    }

}

CmdHandler::CmdHandler(int epollFd)
{
    epoll_event ee {EPOLLIN,{.ptr=this}};
    epoll_ctl(epollFd, EPOLL_CTL_ADD, STDIN_FILENO, &ee);
}

void CmdHandler::listConnected()
{
    for(Client* c : clients)
    {
        printf("Client: %d\n",c->fd());
    }
}

void CmdHandler::listRooms()
{
    for(Room* r : rooms)
    {
        printf("Room: %s\n",r->name().c_str());
    }
}

void CmdHandler::sendTo(int fd, char * buffer, int count)
{
    for(Client*c : clients)
    {
        if(c->fd()==fd)
        {
            c->write(buffer,count);
            break;
        }
    }
}