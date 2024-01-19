#pragma once

#include <iostream>
#include <thread>
#include <chrono>

#include "room.h"

class Room;

extern Room dummy;

struct Timer
{
    bool running;
    bool paused;
    Room &room;
    std::chrono::milliseconds passed;
    std::chrono::milliseconds target;
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> current;
    std::thread timerThread;
    Timer();
    Timer(Room *r);
    ~Timer();
    void timerLoop();
    void startTimer(int ms);
    void pauseTimer();
    void changeTimer(int ms);
    void stopTimer();

};
