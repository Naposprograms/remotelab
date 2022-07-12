#include "timer.h"


Timer::Timer() {} // Nothing to do in constructor

void Timer::set(unsigned long time_elapsed)
{
    timer = millis() + time_elapsed;
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