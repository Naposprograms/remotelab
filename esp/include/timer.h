#include <esp32-hal.h>


/**
 * @brief Simple non-blocking timer object to periodically check if the set time has elapsed.
 */
class Timer {

    private:

        unsigned long timer = 0;

    
    public:

        Timer();
        ~Timer();

        /**
         * @brief Set a timer in ms 
         * 
         * @param time_elapsed Time in ms to count up to.  
         */
        void set(unsigned long time_elapsed);

        /**
         * @brief Call this method periodically to check if the set time has elapsed
         * 
         * @return true if set time has elapsed.
         * @return false if set time has not elapsed.
         */
        bool elapsed();

};