#pragma once
#include <iostream>
#include <unordered_map>
#include <random>
#include <fstream>
#include <algorithm>
#include "handler.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

class Client;

struct Answers
{
    std::unordered_map<std::string,std::vector<Client*>> country;
    std::unordered_map<std::string,std::vector<Client*>> city;
    std::unordered_map<std::string,std::vector<Client*>> name;
};

bool findInCSV(std::string path,std::string answer);
void toUpper(std::string &str);

class Room
{
    bool _inGame;
    int _maxPlayers;
    int _currentPlayers;
    int _finished;
    int _MaxRounds;
    std::string _name;
    std::vector<char> l;
    std::string _letter;
    std::unordered_set<Client*> players;
    Answers answers;
    public:
        Room(std::string name,int maxPlayers=4);
        std::string name();
        bool inGame();
        int maxPlayers();
        int currentPlayers();
        void listPlayers();
        void removePlayer(Client* c);
        void sendCurrentPlayers(Client* c);
        void join(Client* c);
        void sendToAllInRoomBut(Client* player, std::string msg);
        void sendToAllInRoom(std::string msg);
        std::string getRandomLatter();
        void startRound(std::string letter);
        void startGame();
        void printAnswers();
        void submitAnswer(Client* c,std::string &country,std::string &city,std::string &name);
        void scorePlayers();
};


