#include "room.h"

Room::Room(std::string name,int maxPlayers): _name(name), _maxPlayers(maxPlayers)
{
    _inGame=false;
    players={};
}

std::string Room::name()
{
    return _name;
}

bool Room::inGame()
{
    return _inGame;
}

int Room::maxPlayers()
{
    return _maxPlayers;
}

int Room::currentPlayers()
{
    return _currentPlayers;
}

void Room::listPlayers()
{
    for(Client* p : players)
    {
        printf("Player: %s %d\n",p->name(),p->fd());
    }
}

void Room::sendCurrentPlayers(Client* c)
{
    std::stringstream ss;
    ss<<"CURRENTPLAYERS|";
    for(Client* p : players)
    {
        ss<<p->name()<<"|";
    }
    std::string tmp = ss.str();
    std::cout << tmp<<std::endl;
    c->write((char *)tmp.c_str(),tmp.length());
}

void Room::join(Client* c)
{
    if(!(_currentPlayers<_maxPlayers))
    {
        c->write("ROOMFULL\n",10);
        return;
    }
    std::stringstream ss;
    ss<<"JOINED|"<<c->name()<<"\n";
    std::string tmp = ss.str();
    std::cout << tmp<<std::endl;
    sendToAllInRoomBut(c,tmp);
    c->write((char *)tmp.c_str(),tmp.length());
    sendCurrentPlayers(c);
    players.insert(c);
    _currentPlayers++;
}

void Room::sendToAllInRoomBut(Client* player, std::string msg)
{
    for(Client* p : players)
    {
        if(p==player)
        {
            continue;
        }
        p->write((char*)msg.c_str(),msg.length());
    }
}