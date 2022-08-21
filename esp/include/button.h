#ifndef BUTTON_H
#define BUTTON_H


/**
 * @brief Simple non-blocking button object to periodically check if the a button has been pressed.
 * For a normal button, it is required to call release in order to detect a new push.
 * For a timed button, after the defined time, it can ben pressed again.
 * release() will also work with a timed button, to release before the timer has elapsed. 
 */


#include <esp32-hal.h>
#include <timer.h>


#define ACTIVE_LOW 0
#define ACTIVE_HIGH 1


class Button {

    private:

        uint8_t buttonPin;
        bool logic;
        bool buttonPressed;


    protected:

        bool buttonBlocked;
    

    public:
        /**
         * @brief Construct a new Button object
         * 
         * @param pin The microcontroller pin where the button signal will be connected.
         * @param positiveLogic true if the voltage is Vcc/Vss when pressed (rising edge),
         *                      false if the voltage is GND when pressed (falling edge).
         */
        Button(uint8_t pin, bool positiveLogic);

        /**
         * @brief  Returns true when button is pressed. Blocks new pushes until release method is called.
         *          This method must be called periodically to detect pushes.
         * 
         * @return true is the button was pressed.
         * @return false is the button has not been pressed.
         */
        bool pressed();
        
        /**
         * @brief Releases the button to be able to detect new pushes.
         * 
         */
        void release();

};

class TimedButton : protected Button {

    private:

        uint32_t msToRelease;
        Timer buttonTimer;


    public:
        /**
         * @brief Construct a new Timed Button object
         * 
         * @param pin The microcontroller pin where the button signal will be connected.
         * @param positiveLogic true if the voltage is Vcc/Vss when pressed (rising edge),
         *                      false if the voltage is GND when pressed (falling edge).
         * @param releaseTime The time in ms upon which the program will be available to detect new pushes. 
         */
        TimedButton(uint8_t pin, bool positiveLogic, uint32_t releaseTime);

        /**
         * @brief Returns true when button is pressed. Blocks new pushes until release method is called.
         *          This method must be called periodically to detect pushes.
         * 
         * @return true is the button was pressed.
         * @return false is the button has not been pressed.
         */
        bool pressed();

        /**
         * @brief Releases the button to be able to detect new pushes.
         * 
         */
        void release();
};

#endif