#ifndef COSPHI_H
#define COSPHI_H


// module tested ok on inductive. missing documentation & testing capacitive and resitive.
#include <Arduino.h>


#define LINE_FREQUENCY_IN_HZ 50.0
#define PERIOD_IN_MS 1000 * (1.0 / LINE_FREQUENCY_IN_HZ)


class CosPhi {

    private:

        bool triggerSampling = false;
        bool valueReady = false;
        uint8_t voltageSignalPin;
        uint8_t currentSignalPin;

        /* Detects if the circuit is inductive or capacitive (to know which signal comes first).
        Since only the falling edge is being detected 
        (which is when the signal crosses from negative to positive),
        if there is more than half a period between the signals then we have read first the last signal.
        Therefore we assume that the opposite signal comes first, and then read the delay. */
        bool validation();

    public:

        CosPhi(uint8_t voltageZeroCrossSignalPin, uint8_t currentZeroCrossSignalPin);

        bool task();

        void commandSampling();

        float getCosPhi();
};

#endif