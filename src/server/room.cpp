#include "room.h"

Room::Room(std::string name,int maxPlayers,int LastRound): _name(name), _maxPlayers(maxPlayers), _LastRound(LastRound)
{
    for(int i=65;i<=90;i++)
    {
        l.push_back(i);
    }
    _currentPlayers=0;
    _finished=0;
    _inGame=false;
    _Round=0;
    if(_LastRound>5)
    {
        _LastRound=5;
    }
    players={};
}
Room::Room()
{

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
    ss<<"CURRENTPLAYERS|";
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
    if(_Round<_LastRound)
    {
        _finished=0;
        _Round++;
        sendToAllInRoom("START|"+letter+"\n");
        std::cout << "START|"+letter+"\n";
        return;
    }
    std::cout << "END" <<std::endl;
    sendToAllInRoom("END\n");
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
    sendToAllInRoom("THANKS|"+c->name()+"\n");
    if(_finished==players.size())
    {
        std::cout << "Scoring" << std::endl;
        scorePlayers();
        startRound(getRandomLatter());
        _finished = 0;;
    }
    else
    {
        std::cout << "Waiting for more answers..." << std::endl;
    }

}

//Verry inefficient on memory ngl
void Room::scorePlayers()
{
    std::vector<std::string> validCountry;
    //std::vector<std::string> invalidCountry;
    std::vector<std::string> validCity;
    //std::vector<std::string> invalidCity;
    std::vector<std::string> validName;
    //std::vector<std::string> invalidName;
    for(auto& [key,vec]:answers.country)
    {
        if(findInCSV("countries.csv",key))
        {
            validCountry.push_back(key);
        }
        else
        {
            //invalidCountry.push_back(key);
        }
    }
    if(validCountry.size()==1&&(answers.country[validCountry[0]].size()==1))
    {
        answers.country[validCountry[0]][0]->addPoints(15);
    }
    else
    {
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
                //c->showPoints();
            }
        }
    }

    for(auto& [key,vec]:answers.city)
    {
        if(findInCSV("cities.csv",key))
        {
            validCity.push_back(key);
        }
        else
        {
            //invalidCountry.push_back(key);
        }
    }
    if(validCity.size()==1&&(answers.city[validCity[0]].size()==1))
    {
        answers.city[validCity[0]][0]->addPoints(15);
    }
    else
    {
        for(std::string key : validCity)
        {
            for(Client* c : answers.city[key])
            {
                if(answers.city[key].size()==1)
                {
                    c->addPoints(10);
                }
                else
                {
                    c->addPoints(5);
                }
                //c->showPoints();
            }
        }
    }
    
    for(auto& [key,vec]:answers.name)
    {
        if(findInCSV("names.csv",key))
        {
            validName.push_back(key);
        }
        else
        {
            //invalidCountry.push_back(key);
        }
    }
    if(validName.size()==1&&(answers.name[validName[0]].size()==1))
    {
        answers.name[validName[0]][0]->addPoints(15);
    }
    else
    {
        for(std::string key : validName)
        {
            for(Client* c : answers.name[key])
            {
                if(answers.name[key].size()==1)
                {
                    c->addPoints(10);
                }
                else
                {
                    c->addPoints(5);
                }
                //c->showPoints();
            }
        }
    }

    for(Client * c : players)
    {
        //c->showPoints();
        //c->write();
        sendToAllInRoom("SCORES|"+c->name()+"|"+std::to_string(c->points())+"\n");
    }
}


bool Room::findInCSV(std::string path,std::string answer)
{
    toUpper(answer);
    if(answer[0]!=_letter[0])
    {
        return false;
    }
    if(answer=="YYY")
    {
        return false;
    }
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

bool findInCSV(std::string path,std::string answer)
{
    toUpper(answer);
    std::ifstream in(path);
    std::string tmp;
    std::getline(in,tmp,'\n');//skip first list
    std::getline(in,tmp,'\n');
    while(!in.eof())
    {
        if(tmp[0]!=answer[0])
        {
            std::getline(in,tmp,'\n');
            continue;
        }
        while((!in.eof())&&(tmp[0]==answer[0]))
        {
            toUpper(tmp);
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
