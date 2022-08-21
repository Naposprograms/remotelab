#include <adc.h>


// volatile keyword should be used in the definition of a variable
// when the variable's value could change outside the normal flow of execution of the program.
volatile bool newData;

// must be global to attach to interrupt
#ifndef ESP32
    void newDataReadyADC()
#else
    void IRAM_ATTR newDataReadyADC()
#endif
{
    newData = true;
}


Adc::Adc(uint8_t adcAlertPin, uint8_t calibrationSignalPin)
{
    alertPin = adcAlertPin;
    calibrationPin = calibrationSignalPin;
}


void Adc::routeSample()
{
    int16_t sampleValue = moduleADC.getLastConversionResults();
    if(currentSampling != CALIBRATION)
    {
        if(currentSamplingIsDifferential)
        {
            switch (currentChannel)
            {
                #define X(varName, chNumber)                    \
                    case chNumber:                              \
                        if(currentSampling == SINGLE_SAMPLE)    \
                        { varName.singleSample = sampleValue; } \
                        if(currentSampling == MANY_SAMPLES)     \
                        { * varName.bufferPtr = sampleValue;    \
                            varName.bufferPtr++; }              \
                        break;
                    DIFF_CHANNELS
                #undef X
                    case 3:
                        if(currentSampling == SINGLE_SAMPLE)
                        { chDiff01.singleSample = sampleValue; }
                        if(currentSampling == MANY_SAMPLES)
                        { * chDiff01.bufferPtr = sampleValue;
                            chDiff01.bufferPtr++; }
                        break;
                default:
                    break;
            }
        }
        else
        {
            switch (currentChannel)
            {
                #define X(varName, chNumber)                    \
                    case chNumber:                              \
                        if(currentSampling == SINGLE_SAMPLE)    \
                        { varName.singleSample = sampleValue; } \
                        if(currentSampling == MANY_SAMPLES)     \
                        { * varName.bufferPtr = sampleValue;    \
                            varName.bufferPtr++; }              \
                        break;
                    CHANNELS
                #undef X

                default:
                    break;
            }
        }
    }
    else
    {
        switch (currentChannel)
        {
            #define X(varName, chNumber)                    \
                case chNumber:                              \
                    Serial.printf("CH%d, bits:%d\n", chNumber, sampleValue);\
                    varName.offsetBits = sampleValue;       \
                    break;
                CHANNELS
            #undef X

            default:
                break;
        }
    }
}


bool Adc::begin()
{
    bool moduleOk = moduleADC.begin();
    if(moduleOk)
    {
        pinMode(alertPin, INPUT);
        pinMode(calibrationPin, OUTPUT);
        digitalWrite(calibrationPin, LOW);
        attachInterrupt(digitalPinToInterrupt(alertPin), newDataReadyADC, FALLING);

        moduleADC.setDataRate(RATE_ADS1115_860SPS); // see Adafruit_ADS1X15.h
        moduleADC.setGain(GAIN_TWO); // see Adafruit_ADS1X15.h
        
        missingSamples = 0;
        currentSamplingIsDifferential = false;
        samplingDone = true;
        newData = false;
    }
    return moduleOk;
}


bool Adc::task()
{
    if(currentSampling)
    {
        if(newData)
        {
            switch (currentSampling)
            {
                case SINGLE_SAMPLE:
                    routeSample();
                    samplingDone = true;
                    newData = false;
                    currentSampling = NONE;
                    break;
                
                case MANY_SAMPLES:
                    routeSample();
                    missingSamples--;
                    newData = false;
                    if(!missingSamples)
                    {
                        samplingDone = true;
                        currentSampling = NONE;
                        if(currentSamplingIsDifferential)
                        {
                            switch (currentChannel)
                            {
                                #define X(varName, chNumber)                            \
                                    case chNumber:                                      \
                                        varName.bufferPtr = &varName.samplesBuffer[0];  \
                                        break;
                                    DIFF_CHANNELS
                                #undef X

                                    case 3:
                                        chDiff01.bufferPtr = &chDiff01.samplesBuffer[0];
                                        break;
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            switch (currentChannel)
                            {
                                #define X(varName, chNumber)                            \
                                    case chNumber:                                      \
                                        varName.bufferPtr = &varName.samplesBuffer[0];  \
                                        break;
                                    CHANNELS
                                #undef X

                                default:
                                    break;
                            }
                        }
                    }
                    break;

                case CALIBRATION:
                    routeSample();
                    missingSamples--;
                    newData = false;
                    if(missingSamples)
                    {
                        currentChannel--;
                        switch (currentChannel)
                        {
                            #define X(varName, chNumber)                                    \
                                case chNumber:                                              \
                                    moduleADC.startADCReading(                              \
                                        ADS1X15_REG_CONFIG_MUX_SINGLE_##chNumber, false);   \
                                    break;
                                CHANNELS
                            #undef X

                            default:
                                break;
                        }
                    }
                    else
                    {
                        digitalWrite(calibrationPin, LOW);
                        samplingDone = true;
                        currentSampling = NONE;
                    }                    
                    break;

                default:
                    break;
            }
        }
    }
    return samplingDone;
}


void Adc::commandSampling(uint8_t channel, uint8_t numberOfSamples, bool differential)
{
    // will only command a new sampling if it is done with the previous one
    if(currentSampling == NONE)
    {
        if(channel >= 0 && channel <= 3)
        {
            bool many = false;
            if(numberOfSamples > 1)
            {
                many = true;
                if(numberOfSamples > MAX_SAMPLES_ARRAY_SIZE)
                {
                    numberOfSamples = MAX_SAMPLES_ARRAY_SIZE;
                }
            }
            if(many)
            {
                currentSampling = MANY_SAMPLES;
                missingSamples = numberOfSamples;
            }
            else
            {
                currentSampling = SINGLE_SAMPLE;
            }
            
            currentChannel = channel;
            
            if(differential)
            {
                currentSamplingIsDifferential = true;

                switch (currentChannel)
                {
                    #define X(varName, chNumber)                                        \
                        case chNumber:                                                  \
                            moduleADC.startADCReading(                                  \
                                ADS1X15_REG_CONFIG_MUX_DIFF_##chNumber##_3, many);      \
                            if(many) { varName.bufferPtr = &varName.samplesBuffer[0]; } \
                            else { varName.bufferPtr = &varName.singleSample; }         \
                            break;
                        DIFF_CHANNELS
                    #undef X
                        case 3:
                            moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_DIFF_0_1, many);
                            if(many) { chDiff01.bufferPtr = &chDiff01.samplesBuffer[0]; }
                            else { chDiff01.bufferPtr = &chDiff01.singleSample; }
                            break;
                    default:
                        break;
                }
            }
            else
            {
                currentSamplingIsDifferential = false;
                switch (currentChannel)
                {
                    #define X(varName, chNumber)                                        \
                        case chNumber:                                                  \
                            moduleADC.startADCReading(                                  \
                                ADS1X15_REG_CONFIG_MUX_SINGLE_##chNumber, many);        \
                            if(many) { varName.bufferPtr = &varName.samplesBuffer[0]; } \
                            else { varName.bufferPtr = &varName.singleSample; }         \
                            break;
                        CHANNELS
                    #undef X

                    default:
                        break;
                }
            }
        }

        newData = false;
        samplingDone = false;
    }
}


int16_t Adc::getLastSingleSample(uint8_t channel, bool differential)
{
    if(currentSampling == NONE)
    {
        if(channel >= 0 && channel <= 3)
        {
            if(differential)
            {
                switch (channel)
                {
                    #define X(varName, chNumber)            \
                        case chNumber:                      \
                            return varName.singleSample;    \
                            break;
                        DIFF_CHANNELS
                    #undef X

                        case 3:
                            return chDiff01.singleSample;
                            break;
                    default:
                        break;
                }
            }
            else
            {
                switch (channel)
                {
                    #define X(varName, chNumber)            \
                        case chNumber:                      \
                            return varName.singleSample;    \
                            break;
                        CHANNELS
                    #undef X

                    default:
                        break;
                }
            }
        }
    }
    return 0; // will return a 0 bit result unless the module is done sampling
}


int16_t * Adc::getLastChannelSamples(uint8_t channel, bool differential)
{
    if(currentSampling == NONE)
    {
        if(channel >= 0 && channel <= 3)
        {
            if(differential)
            {
                switch (channel)
                {
                    #define X(varName, chNumber)            \
                        case chNumber:                      \
                            return varName.bufferPtr;       \
                            break;
                        DIFF_CHANNELS
                    #undef X

                        case 3:
                            return chDiff01.bufferPtr;
                            break;
                    default:
                        break;
                }
            }
            else
            {
                switch (channel)
                {
                    #define X(varName, chNumber)            \
                        case chNumber:                      \
                            return varName.bufferPtr;       \
                            break;
                        CHANNELS
                    #undef X

                    default:
                        break;
                }
            }
        }
    }
    return nullSample; // will return a 0 bit result unless the module is done sampling
}


void Adc::calibrateDCOffset()
{
    // will only command to calibrate if there is no ongoing sampling task
    if(currentSampling == NONE)
    {
        digitalWrite(calibrationPin, HIGH);
        // correct this with a non-blocking timer object
        delay(500); // delays to allow BJTs to ground inputs and caps to discharge before starting to sample
        currentChannel = 3;
        // starts by channel 3, then in the task it goes descending to channel 0 
        moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_3, false);
        missingSamples = 4;
        newData = false;
        samplingDone = false;
        currentSampling = CALIBRATION;
    }
}


float Adc::convertBitsToVoltage(int16_t bits, float signalAmplitudeFactor)
{
    return signalAmplitudeFactor * ( (float) bits / FULL_SCALE_BITS ) * FULL_SCALE_VOLTAGE;
}


float Adc::convertBitsToVoltageWithDCOffset(int16_t bits, float signalAmplitudeFactor, uint8_t adcChannel)
{
    float offsetVoltage;
    int16_t offsetInBits;
    switch (adcChannel)
    {
        #define X(varName, chNumber)                \
            case chNumber:                          \
                offsetInBits = varName.offsetBits;  \
                break;
            CHANNELS
        #undef X

        default:
            break;
    }
    offsetVoltage = convertBitsToVoltage(offsetInBits, 1);
    return signalAmplitudeFactor * ((offsetVoltage * (bits) / offsetInBits) - offsetVoltage);
}