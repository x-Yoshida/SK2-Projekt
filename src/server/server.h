#pragma once
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unordered_set>
#include <list>
#include <signal.h>
#include <pthread.h>

#include "handler.h"


std::unordered_set<Client*> clients;
std::unordered_set<Room*> rooms;
Server* servHandler;
Room dummy;

void ctrl_c(int);

void sendToAllBut(int fd, char * buffer, int count);

uint16_t readPort(char * txt);

void setReuseAddr(int sock);

void* connectionCheck(void* arg);


