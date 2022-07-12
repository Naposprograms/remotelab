#include <button.h>


Button::Button(uint8_t pin, bool positiveLogic)
{
    buttonPin = pin;
    logic = positiveLogic;
    buttonPressed = false;
    buttonBlocked = false;
    if(logic)
    {
        pinMode(buttonPin, INPUT_PULLDOWN);
    }
    else
    {
        pinMode(buttonPin, INPUT_PULLUP);
    }
}

bool Button::pressed()
{
    if(!buttonBlocked)
    {
        if(digitalRead(buttonPin) == logic)
        {
            buttonPressed = true;
            if(buttonPressed)
            {
                buttonBlocked = true;
                buttonPressed = false;
                // the only time it will return true is when is this one iteration
                return buttonBlocked;
            }
        }
    }
    return buttonPressed;
}

void Button::release()
{
    buttonBlocked = false;
}

TimedButton::TimedButton(uint8_t pin, bool positiveLogic, uint32_t releaseTime) : Button(pin, positiveLogic)
{
    msToRelease = releaseTime;
    buttonTimer.set(0);
}

bool TimedButton::pressed()
{
    if(!buttonBlocked)
    {
        if(Button::pressed())
        {
            buttonBlocked = true;
            buttonTimer.set(msToRelease);
            return true;
        }
    }
    else
    {
        if(buttonTimer.elapsed())
        {
            buttonBlocked = false;
        }
    }

    return false;
}

void TimedButton::release()
{
    Button::release();
}