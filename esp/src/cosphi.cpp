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
    pinMode(voltageSignalPin, INPUT);
    pinMode(currentSignalPin, INPUT);
}


bool CosPhi::validation()
{
    bool validMeassure = true;
    int32_t difference = microsCurrent - microsVoltage;
    /*
        Possibilities:
        For an inductive circuit difference > 0
        -> (it's correct)
        For an inductive circuit difference < 0
        -> (detected current signal from cycle previous to the voltage signal)
        For a capacitive circuit difference > 0
        -> (detected voltage signal from cycle previous to the current signal)
        For a capacitive circuit difference < 0
        -> (it's correct)
        
        Since only rising 0 crosses are detected, if a difference is higher in time than
        a quarter period (90º phase shift), then the signals detected belong to different cycles.
        Therefore, the meassure must be taken again until a difference < 1/4 T is found.
        Finally the * 1000 factor is because the difference is expressed in microseconds.
    */
    if(abs(difference) > PERIOD_IN_MS * 0.25 * 1000)
    {
        validMeassure = false;
    }
    return validMeassure;
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
        
        validation() ? valueReady = true : triggerSampling = true;
        // add something to prevent loop stuck in triggerSampling if the signal never shows
        // i.e. if there is nothing connected
    }

    return valueReady;
}


void CosPhi::commandSampling()
{
    triggerSampling = true;
}


float CosPhi::getCosPhi()
{
    // see "error" return value if the measure fails (might be higher than 1 for example)
    valueReady = false;
    return cos(2 * PI * ( (microsCurrent - microsVoltage) / (1000 * PERIOD_IN_MS)));
}