#ifndef TIMER_H
#define TIMER_H


/**
 * @brief Simple non-blocking timer object to periodically check if the set time has elapsed.
 */


#include <esp32-hal.h>


class Timer {

    private:

        unsigned long timer = 0;

    
    public:
        /**
         * @brief Construct a new Timer object
         * 
         */
        Timer();

        /**
         * @brief Set a timer in ms 
         * 
         * @param timeToPass Time in ms to count up to.  
         */
        void set(unsigned long timeToPass);

        /**
         * @brief Call this method periodically to check if the set time has elapsed
         * 
         * @return true if set time has elapsed.
         * @return false if set time has not elapsed.
         */
        bool elapsed();

};

#endif