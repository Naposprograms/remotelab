#ifndef COSPHI_H
#define COSPHI_H


#include <Arduino.h>


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
        void circuitReactiveCharacteristic();

    public:

        CosPhi(uint8_t voltageZeroCrossSignalPin, uint8_t currentZeroCrossSignalPin);

        bool task();

        void commandSampling();

        float getCosPhi();
};

#endif