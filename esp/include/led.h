#include <timer.h>
#include <esp32-hal-gpio.h>


/**
 * @brief Easy LED handler module for cleaner code in main application.
 * It allows to create an alive type LED and a signal type LED.
 * Alive type LED blinks permanently to indicate that the microcontroller is working.
 * Signal type LED blinks on command the specified number of times to signal an specific part of the program. 
 */


class Led {

    private:

        bool aliveType, ledOn;
        uint32_t timeOn, timeOff;
        uint8_t ledPin;
        Timer ledTimer;


    protected:

        bool blinking;
        uint8_t toBlink;

        /**
         * @brief Set the time On and Off of the LED.
         * 
         * @param msOn Time on (ms).
         * @param msOff Time off (ms).
         */
        void setOnOffTimes(uint32_t msOn, uint32_t msOff);


    public:

        /**
         * @brief Base class for LED types.
         * 
         * @param pin The microcontroller pin number which is connected to a the LED.
         * @param aliveType true to blink permanently (alive), false to blink on command (a signal).
         */
        Led(uint8_t pin, bool isAliveType);
        ~Led();

        void taskLed();


};


class AliveLed : private Led {

    public:
        /**
         * @brief Create an alive type LED indicating how often it should turn on and for how long.
         * 
         * @param timeOn The time (ms) that the LED has to be ON.
         * @param timeOff The time (ms) that the LED has to be OFF.
         * @param pin The microcontroller pin number which is connected to the LED.
         */
        AliveLed(uint8_t pin, uint32_t timeOn, uint32_t timeOff);
        ~AliveLed();
        void taskAliveLed();

};


class SignalLed : private Led {

    public:
        /**
         * @brief Create a signal type LED indicating for how long it should turn on.
         * 
         * @param timeOn The time (ms) that the LED has to be ON.
         * @param timeOff The time (ms) that the LED has to be OFF before turning ON again.
         * @param pin The microcontroller pin number which is connected to the LED.
         */
        SignalLed(uint8_t pin, uint32_t timeOn, uint32_t timeOff);
        ~SignalLed();

        void taskSignalLed();

        /**
         * @brief Command a signal blink (available to flash many times).
         * 
         * @param blinkingTimes How many times the LED should flash.
         */
        void blink(uint8_t blinkingTimes);

};