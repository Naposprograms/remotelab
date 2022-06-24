#include "timer.h"


// Nothing to do in constructor and destructor
Timer::Timer() {}
Timer::~Timer() {}


void Timer::set(unsigned long time_elapsed)
{
    timer = millis() + time_elapsed;
}

bool Timer::elapsed()
{
    bool timeElapsed = false;
    
    if( millis() >= timer )
    {
        timeElapsed = true;
    }

    return(timeElapsed);
}