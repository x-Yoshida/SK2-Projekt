#include "timer.h"

Timer::Timer(): room(dummy)
{

}


Timer::Timer(Room *r): room(*r)
{
    running = true;
    paused = true;
    timerThread = std::thread(&Timer::timerLoop,this);
}

Timer::~Timer()
{
    if(timerThread.joinable())
        timerThread.join();
}


void Timer::startTimer(int ms)
{
    target = std::chrono::milliseconds(ms);
    start = std::chrono::system_clock::now();
    paused = false;

}

void Timer::pauseTimer()
{
    paused = true;
}

void Timer::changeTimer(int ms)
{
    paused = true;
    target = std::chrono::milliseconds(ms);
    start = std::chrono::system_clock::now();
    paused = false;

}

void Timer::timerLoop()
{
    while(running)
    {
        if(!paused)///Tu powinien być mutex prawdopodnie
        {
            current = std::chrono::system_clock::now();
            passed = std::chrono::duration_cast<std::chrono::milliseconds>(current - start);
            if(passed>=target)
            {
                std::cout << passed.count() << std::endl;
                running = false;
                break;
            }
        }
    }
}