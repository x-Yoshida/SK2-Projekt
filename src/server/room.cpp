#include "room.h"

Room::Room(std::string name,int maxPlayers): _name(name), _maxPlayers(maxPlayers)
{
    for(int i=65;i<=90;i++)
    {
        l.push_back(i);
    }
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
    c->write(tmp);
}

void Room::join(Client* c)
{
    if(!(_currentPlayers<_maxPlayers))
    {
        c->write("ROOMFULL\n");
        return;
    }
    c->joinRoom(this);
    std::stringstream ss;
    ss<<"JOINED|"<<c->name()<<"\n";
    std::string tmp = ss.str();
    std::cout << tmp<<std::endl;
    sendToAllInRoomBut(c,tmp);
    c->write(tmp);
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
        p->write(msg);
    }
}

void Room::sendToAllInRoom(std::string msg)
{
    for(Client* p : players)
    {
        p->write(msg);
    }
}

std::string Room::getRandomLatter()
{
    std::string letter;
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0,l.size()-1);
    int i=dist(rd);
    letter = l[i];
    l.erase(l.begin()+i);
    return letter;
}

void Room::startRound(std::string letter)
{
    sendToAllInRoom("START|"+letter+"\n");
    std::cout << "START|"+letter+"\n";
}

void Room::startGame()
{
    startRound(getRandomLatter());
    _inGame=true;
}

void Room::printAnswers()
{
    for(auto& [key,vec] : answers.country)
    {
        std::cout << key << " ";
        for(Client* c : vec)
        {
            std::cout << c->fd() << " ";
        }
        std::cout << std::endl;
    }
    for(auto& [key,vec] : answers.city)
    {
        std::cout << key << " ";
        for(Client* c : vec)
        {
            std::cout << c->fd() << " ";
        }
        std::cout << std::endl;
    }
    for(auto& [key,vec] : answers.name)
    {
        std::cout << key << " ";
        for(Client* c : vec)
        {
            std::cout << c->fd() << " ";
        }
        std::cout << std::endl;
    }
    answers.name.clear();
}

void Room::submitAnswer(Client* c,std::string &country,std::string &city,std::string &name)
{
    answers.country[country].push_back(c);
    answers.city[city].push_back(c);
    answers.name[name].push_back(c);
    
}