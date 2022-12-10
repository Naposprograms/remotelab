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

        Given that the formula subtracts the voltage timeframe from the current timeframe,
        if the result is negative but it is in the quarter period range from the previous period
        it can be corrected by subtracting a whole period of the timeframe from the voltage signal.
    */
    if(abs(difference) > quarterPeriodMicroSeconds)
    {
        Serial.printf("uI = %d uV = %d\n", microsCurrent, microsVoltage);
        if(negativePeriodMicroSeconds < difference && difference < negativePeriodMicroSeconds + quarterPeriodMicroSeconds)
        {
            microsVoltage -= PERIOD_IN_MS * 1000;
            difference = microsCurrent - microsVoltage;
        }
        else
        {
            if(-1 * negativePeriodMicroSeconds < difference && difference < -1 * (negativePeriodMicroSeconds + quarterPeriodMicroSeconds))
            {
                microsCurrent -= PERIOD_IN_MS * 1000;
                difference = microsCurrent - microsVoltage;
            }
            else
            {
                Serial.printf("Invalid cosphi meassure. Difference = %d\n", difference);
                validMeassure = false;
                attempts++;
            }
        }
    }

    difference > 0 ? meassuredInductive = true : meassuredInductive = false;
    
    return validMeassure;
}


bool CosPhi::task()
{
    if(triggerSampling)
    {
        triggerSampling = false;
        // if 5 attempts weren't enough then something's wrong with the circuit load.
        // the getCosPhi() method will return 0 
        if(attempts < 5)
        {
            attachInterrupt(digitalPinToInterrupt(voltageSignalPin), zeroCrossVoltage, FALLING);
            attachInterrupt(digitalPinToInterrupt(currentSignalPin), zeroCrossCurrent, FALLING);
        }
        else
        {
            missingLoad = true;
            valueReady = true;
        }
    }
    if(voltageCrossedZero && currentCrossedZero)
    {
        detachInterrupt(voltageSignalPin);
        detachInterrupt(currentSignalPin);
        voltageCrossedZero = false;
        currentCrossedZero = false;
        validation() ? valueReady = true : triggerSampling = true;
    }

    // timer to prevent loop getting stuck if after some time one of the signals is never crossing zero
    // this might be the case if there is no circuit load
    if(meassureTimeOut.elapsed())
    {
        missingLoad = true;
        valueReady = true;
        Serial.println("Cosphi timer elapsed");
    }

    return valueReady;
}


void CosPhi::commandSampling()
{
    triggerSampling = true;
    attempts = 0;
    missingLoad = false;
    meassureTimeOut.set(PERIOD_IN_MS * 5);
}



float CosPhi::getCosPhi()
{
    valueReady = false;
    int16_t diff = microsCurrent - microsVoltage;
    if(missingLoad)
    {
        return 0;
    }
    else
    {
        return cos(2 * PI * (diff / (1000.0 * PERIOD_IN_MS)));
    }
}