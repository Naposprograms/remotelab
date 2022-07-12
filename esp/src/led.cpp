#include <led.h>


Led::Led(uint8_t pin, bool isAliveType)
{
    aliveType = isAliveType;
    ledPin = pin;
    pinMode(ledPin, OUTPUT);
    toBlink = 0;
}

void Led::setOnOffTimes(uint32_t msOn, uint32_t msOff, Timer * ledTimer)
{
    timeOn = msOn;
    timeOff = msOff;
    ledOn = false;
    digitalWrite(ledPin, ledOn);
    ledTimer->set(msOff);
}

void Led::taskLed(Timer * ledTimer)
{
    if(aliveType || toBlink)
    {
        if(ledTimer->elapsed())
        {
            // switch led state
            ledOn = !ledOn;
            digitalWrite(ledPin, ledOn);
            // set new timer
            if(ledOn)
            {
                ledTimer->set(timeOn);
            }
            else
            {
                ledTimer->set(timeOff);
            }
            // manage blinking times
            if(toBlink > 0 && !ledOn)
            {
                blinking = true;
                toBlink--;
                if(toBlink == 0)
                {
                    blinking = false;
                }
            }
        }
    }
}


AliveLed::AliveLed(uint8_t pin, uint32_t timeOn, uint32_t timeOff) : Led(pin, true)
{
    Led::setOnOffTimes(timeOn, timeOff, &aliveLedTimer);
}


SignalLed::SignalLed(uint8_t pin, uint32_t timeOn, uint32_t timeOff) : Led(pin, false)
{
    Led::setOnOffTimes(timeOn, timeOff, &signalLedTimer);
}

void SignalLed::blink(uint8_t blinkingTimes)
{
    if(!Led::blinking) // blocks the blinking if it is already blinking
    {
        Led::toBlink = blinkingTimes;
    }
}

void AliveLed::taskAliveLed()
{
    Led::taskLed(&aliveLedTimer);
}

void SignalLed::taskSignalLed()
{
    Led::taskLed(&signalLedTimer);
}