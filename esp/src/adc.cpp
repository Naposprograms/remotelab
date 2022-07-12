#include <adc.h>


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

// Nothing to do in constructor and destructor
Adc::Adc() {}
Adc::~Adc() {}


void Adc::routeSample()
{
    int16_t sampleValue = moduleADC.getLastConversionResults();

    switch (currentChannel)
    {
        #define X(varName, chNumber)                    \
            case chNumber:                              \
                if(currentSampling == SINGLE_SAMPLE)    \
                {                                       \
                    varName.singleSample = sampleValue; \
                }                                       \
                if(currentSampling == MANY_SAMPLES)     \
                {                                       \
                    * varName.bufferPtr = sampleValue;  \
                    varName.bufferPtr++;                \
                }                                       \
                break;                                  
            CHANNELS
        #undef X

        default:
            break;
    }
}


bool Adc::initiate()
{
    bool moduleOk = moduleADC.begin();
    if(moduleOk)
    {
        moduleADC.setDataRate(RATE_ADS1115_860SPS);
        moduleADC.setGain(GAIN_TWO);
        pinMode(ADC_ALERT_PIN, INPUT);
        attachInterrupt(digitalPinToInterrupt(ADC_ALERT_PIN), newDataReadyADC, FALLING);
        missingSamples = 0;
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
                    break;

                default:
                    break;
            }
        }
    }
    return samplingDone;
}

int16_t * Adc::getLastChannelSamples(uint8_t channel)
{
    if(currentSampling == NONE)
    {
        if(channel >= 0 && channel <= 3)
        {
            switch (channel)
            {
                #define X(varName, chNumber)            \
                    case chNumber:                      \
                        return varName.bufferPtr; \
                        break;
                    CHANNELS
                #undef X

                default:
                    return nullSample;
                    break;
            }
        }
    }
    else
    {
        return nullSample; // will return a 0 bit result unless the module is done sampling
    }

}

void Adc::commandSampling(uint8_t channel, bool many)
{
    // will only command a new sampling if it is done with the previous one
    if(currentSampling == NONE)
    {
        if(many)
        {
            currentSampling = MANY_SAMPLES;
            missingSamples = SAMPLES_ARRAY_SIZE;
        }
        else
        {
            currentSampling = SINGLE_SAMPLE;
        }

        if(channel >= 0 && channel <= 3)
        {
            currentChannel = channel;
            switch (currentChannel)
            {
                #define X(varName, chNumber)                                                        \
                    case chNumber:                                                                  \
                        moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_##chNumber, many);  \
                        if(many) { varName.bufferPtr = &varName.samplesBuffer[0]; }                 \
                        else { varName.bufferPtr = &varName.singleSample; }                         \
                        break;
                    CHANNELS
                #undef X

                default:
                    break;
            }
        }

        newData = false;
        samplingDone = false;
    }
}

void Adc::stopSampling()
{
    currentSampling = NONE;
}