#pragma once
#include <iostream>

#include "handler.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

class Client;

class Room
{
    bool _inGame;
    int _maxPlayers;
    int _currentPlayers;
    std::string _name;
    std::unordered_set<Client*> players;
    public:
        Room(std::string name,int maxPlayers=4);
        std::string name();
        bool inGame();
        int maxPlayers();
        int currentPlayers();
        void listPlayers();
        void sendCurrentPlayers(Client* c);
        void join(Client* c);
        void sendToAllInRoomBut(Client* player, std::string msg);

};