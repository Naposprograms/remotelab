#ifndef LED_H
#define LED_H


/**
 * @brief Easy LED handler module for cleaner code in main application.
 * It allows to create an alive type LED and a signal type LED.
 * Alive type LED blinks permanently to indicate that the microcontroller is working.
 * Signal type LED blinks on command the specified number of times to signal an specific part of the program.
 * The hardware connection must be positive logic (the output when ON is Vcc/Vss and OFF is GND).
 */


#include <timer.h>
#include <esp32-hal-gpio.h>


class Led {

    private:

        bool aliveType, ledOn;
        uint32_t timeOn, timeOff;
        uint8_t ledPin;


    protected:

        bool blinking;
        uint8_t toBlink;

        /**
         * @brief Set the time On and Off of the LED.
         * 
         * @param msOn Time on (ms).
         * @param msOff Time off (ms).
         */
        void setOnOffTimes(uint32_t msOn, uint32_t msOff, Timer * ledTimer);
        void taskLed(Timer * ledTimer);


    public:
        /**
         * @brief Base class for LED types.
         * 
         * @param pin The microcontroller pin number which is connected to a the LED.
         * @param aliveType true to blink permanently (alive), false to blink on command (a signal).
         */
        Led(uint8_t pin, bool isAliveType);

};


class AliveLed : private Led {

    private:

        Timer aliveLedTimer;


    public:
        /**
         * @brief Create an alive type LED indicating how often it should turn on and for how long.
         * 
         * @param timeOn The time (ms) that the LED has to be ON.
         * @param timeOff The time (ms) that the LED has to be OFF.
         * @param pin The microcontroller pin number which is connected to the LED.
         */
        AliveLed(uint8_t pin, uint32_t timeOn, uint32_t timeOff);
        
        /**
         * @brief The task must be periodically called in the loop.
         * 
         */
        void taskAliveLed();

};


class SignalLed : private Led {

    private:
        
        Timer signalLedTimer;


    public:
        /**
         * @brief Create a signal type LED indicating for how long it should turn on.
         * 
         * @param timeOn The time (ms) that the LED has to be ON.
         * @param timeOff The time (ms) that the LED has to be OFF before turning ON again.
         * @param pin The microcontroller pin number which is connected to the LED.
         */
        SignalLed(uint8_t pin, uint32_t timeOn, uint32_t timeOff);

        /**
         * @brief The task must be periodically called in the loop.
         * 
         */
        void taskSignalLed();

        /**
         * @brief Command a signal blink (available to flash many times).
         * 
         * @param blinkingTimes How many times the LED should flash.
         */
        void blink(uint8_t blinkingTimes);

};

#endif