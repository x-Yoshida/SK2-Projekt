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

void Room::removePlayer(Client* c)
{
    players.erase(c);
    _currentPlayers--;
}

void Room::sendCurrentPlayers(Client* c)
{
    std::stringstream ss;
    ss<<"CURRENTPLAYERS|";
    for(Client* p : players)
    {
        ss<<p->name()<<"|";
    }
    ss<<"\n";
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
    ss<<"\nCURRENTPLAYERS|";
    for(Client* p : players)
    {
        ss<<p->name()<<"|";
    }
    ss<<"\n";
    tmp = ss.str();
    std::cout << tmp<<std::endl;
    c->write(tmp);
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
    //std::string letter;
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0,l.size()-1);
    int i=dist(rd);
    _letter = l[i];
    l.erase(l.begin()+i);
    return _letter;
}

void Room::startRound(std::string letter)
{
    _finished=0;
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
    _finished++;
    if(_finished==players.size())
    {
        scorePlayers();
    }
}

void Room::scorePlayers()
{
    std::vector<std::string> validCountry;
    std::vector<std::string> invalidCountry;
    std::vector<std::string> validCity;
    std::vector<std::string> invalidCity;
    std::vector<std::string> validName;
    std::vector<std::string> invalidName;
    for(auto& [key,vec]:answers.country)
    {
        if(findInCSV("countries.csv",key))
        {
            validCountry.push_back(key);
        }
        else
        {
            invalidCountry.push_back(key);
        }
    }

    for(std::string key : validCountry)
    {
        for(Client* c : answers.country[key])
        {
            if(answers.country[key].size()==1)
            {
                c->addPoints(10);
            }
            else
            {
                c->addPoints(5);
            }
            c->showPoints();
        }
    }

}


bool findInCSV(std::string path,std::string answer)
{
    toUpper(answer);
    std::ifstream in(path);
    std::string tmp;
    std::getline(in,tmp,'\n');//skip first list
    std::getline(in,tmp,'\n');
    while(!in.eof())
    {
        //std::cout<<tmp<<std::endl<<" "<<(tmp[0]!=answer[0])<<std::endl;
        //sleep(1);
        if(tmp[0]!=answer[0])
        {
            //std::cout<<(tmp[0]!=answer[0])<<std::endl;
            std::getline(in,tmp,'\n');
            continue;
        }
        while((!in.eof())&&(tmp[0]==answer[0]))
        {
            toUpper(tmp);
            //std::cout << tmp << " " << answer << std::endl;
            if(tmp==answer)
            {
                in.close();
                return true;
            }
            std::getline(in,tmp,'\n');
        }
        break;
    }
    in.close();
    return false;
}

void toUpper(std::string &str)
{
    for(int i=0;i<str.length();i++)
    {
        str[i]=toupper(str[i]);
    }
    //std::cout << str << std::endl;
}