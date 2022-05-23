#include <Arduino.h>
#include <config.h>
#include <led.h>


AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);


void setup()
{
    // blink 3 times on startup.
    ledSignal.blink(3);
}

void loop()
{
    ledAlive.taskAliveLed();
    ledSignal.taskSignalLed();
}