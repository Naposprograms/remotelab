#ifndef COSPHI_H
#define COSPHI_H


// module tested ok on inductive and resistive. missing documentation & testing capacitive.
#include <Arduino.h>
#include <timer.h>


#define LINE_FREQUENCY_IN_HZ 50.0
#define PERIOD_IN_MS 1000 * (1.0 / LINE_FREQUENCY_IN_HZ)


class CosPhi {

    private:

        bool triggerSampling = false;
        bool valueReady = false;
        uint8_t voltageSignalPin;
        uint8_t currentSignalPin;
        bool missingLoad = false;
        u_int8_t attempts = 0;
        const int32_t quarterPeriodMicroSeconds = 1000 * PERIOD_IN_MS / 4;
        const int32_t negativePeriodMicroSeconds = 1000 * PERIOD_IN_MS * -1;
        Timer meassureTimeOut;
        

        /* Detects if the circuit is inductive or capacitive (to know which signal comes first).
        Since only the falling edge is being detected 
        (which is when the signal crosses from negative to positive),
        if there is more than half a period between the signals then we have read first the last signal.
        Therefore we assume that the opposite signal comes first, and then read the delay. */
        bool validation();

    public:

        CosPhi(uint8_t voltageZeroCrossSignalPin, uint8_t currentZeroCrossSignalPin);

        bool task();

        void commandSampling(uint16_t delayTimeout);

        float getCosPhi();

        bool meassuredInductive = false;

        void setCosPhiValue(int16_t currentToVoltageLagms, bool inductive);

};

#endif