#include <cosphi.h>


// must be global to attach to interrupt
volatile unsigned long microsVoltage, microsCurrent;
volatile bool voltageCrossedZero, currentCrossedZero = false;

void IRAM_ATTR zeroCrossVoltage()
{
    if(!voltageCrossedZero) // to read only once
    {
        voltageCrossedZero = true;
        microsVoltage = micros();
    }
}

void IRAM_ATTR zeroCrossCurrent()
{
    if(!currentCrossedZero) // to read only once
    {
        currentCrossedZero = true;
        microsCurrent = micros();
    }
}


CosPhi::CosPhi(uint8_t voltageZeroCrossSignalPin, uint8_t currentZeroCrossSignalPin)
{
    voltageSignalPin = voltageZeroCrossSignalPin;
    currentSignalPin = currentZeroCrossSignalPin;
}


bool CosPhi::task()
{
    if(triggerSampling)
    {
        triggerSampling = false;
        attachInterrupt(digitalPinToInterrupt(voltageSignalPin), zeroCrossVoltage, FALLING);
        attachInterrupt(digitalPinToInterrupt(currentSignalPin), zeroCrossCurrent, FALLING);
    }
    if(voltageCrossedZero && currentCrossedZero)
    {
        detachInterrupt(voltageSignalPin);
        detachInterrupt(currentSignalPin);
        voltageCrossedZero = false;
        currentCrossedZero = false;
        valueReady = true;
    }

    return valueReady;
}


void CosPhi::commandSampling()
{
    triggerSampling = true;    
}


float CosPhi::getCosPhi()
{
    valueReady = false;
    // 20000.0 is the period in microseconds for f = 50 Hz
    return cos(2 * PI * ( (microsCurrent - microsVoltage) / 20000.0));
}