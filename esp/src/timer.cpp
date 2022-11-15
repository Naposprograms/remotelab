#include "timer.h"


Timer::Timer() {} // Nothing to do in constructor

void Timer::set(unsigned long timeToPass)
{
    timer = millis() + timeToPass;
}

bool Timer::elapsed()
{
    bool timeElapsed = false;
    
    if(millis() >= timer)
    {
        timeElapsed = true;
    }
    
    return(timeElapsed);
}