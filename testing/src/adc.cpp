#include <adc.h>


volatile bool newData;

// must be global to attach to interrupt
#ifndef ESP32
    void newDataReadyADC()
#elif
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
        /*
        #define X(varName, chNumber)                \
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
            break;                                  \
        CHANNELS
        #undef X
        */
        
        case 0:
            if(currentSampling == SINGLE_SAMPLE)
            {
                ch0.singleSample = sampleValue;
            }
            if(currentSampling == MANY_SAMPLES)
            {
                * ch0.bufferPtr = sampleValue;
                ch0.bufferPtr++;
            }
            break;
        case 1:
            if(currentSampling == SINGLE_SAMPLE)
            {
                ch1.singleSample = sampleValue;
            }
            if(currentSampling == MANY_SAMPLES)
            {
                * ch1.bufferPtr = sampleValue;
                ch1.bufferPtr++;
            }
            break;
        case 2:
            if(currentSampling == SINGLE_SAMPLE)
            {
                ch2.singleSample = sampleValue;
            }
            if(currentSampling == MANY_SAMPLES)
            {
                * ch2.bufferPtr = sampleValue;
                ch2.bufferPtr++;
            }
            break;
        case 3:
            if(currentSampling == SINGLE_SAMPLE)
            {
                ch3.singleSample = sampleValue;
            }
            if(currentSampling == MANY_SAMPLES)
            {
                * ch3.bufferPtr = sampleValue;
                ch3.bufferPtr++;
            }
            break;
        
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
                            /*
                            #define X(varName, chNumber)    \
                                case chNumber:              \
                                    varName.bufferPtr = &varName.samplesBuffer[0]; \
                                    break;                  \
                            CHANNELS
                            #undef X
                            */
                            
                            case 0:
                                ch0.bufferPtr = &ch0.samplesBuffer[0];
                                break;

                            case 1:
                                ch1.bufferPtr = &ch1.samplesBuffer[0];
                                break;

                            case 2:
                                ch2.bufferPtr = &ch2.samplesBuffer[0];
                                break;

                            case 3:
                                ch3.bufferPtr = &ch3.samplesBuffer[0];
                                break;
                            
                            
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
    if(currentSampling == NONE) // will not return a result unless the module is done sampling
    {
        int16_t * samplesPtr;

        if(channel >= 0 && channel <= 3)
        {
            switch (channel)
            {
                /*
                #define X(varName, chNumber)            \
                    case chNumber:                      \
                        samplesPtr = varName.bufferPtr; \
                        break;                          \
                CHANNELS
                #undef X
                */
                
                case 0:
                    samplesPtr = ch0.bufferPtr;            
                    break;

                case 1:
                    samplesPtr = ch1.bufferPtr;            
                    break;

                case 2:
                    samplesPtr = ch2.bufferPtr;            
                    break;

                case 3:
                    samplesPtr = ch3.bufferPtr;            
                    break;
                
                default:
                    break;
            }
        }
        return samplesPtr;
    }
    else
    {
        return 0;
    }

}

void Adc::commandSampling(uint8_t channel, bool many)
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
            case 0:
                moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/many);
                if(many)
                {
                    ch0.bufferPtr = &ch0.samplesBuffer[0];
                }
                else
                {
                    ch0.bufferPtr = &ch0.singleSample;
                }
                break;

            case 1:
                moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, /*continuous=*/many);
                if(many)
                {
                    ch1.bufferPtr = &ch1.samplesBuffer[0];
                }
                else
                {
                    ch1.bufferPtr = &ch1.singleSample;
                }
                break;

            case 2:
                moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_2, /*continuous=*/many);
                if(many)
                {
                    ch2.bufferPtr = &ch2.samplesBuffer[0];
                }
                else
                {
                    ch2.bufferPtr = &ch2.singleSample;
                }
                break;

            case 3:
                moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_3, /*continuous=*/many);
                if(many)
                {
                    ch3.bufferPtr = &ch3.samplesBuffer[0];
                }
                else
                {
                    ch3.bufferPtr = &ch3.singleSample;
                }
                break;
            
            default:
                break;
        }
    }

    newData = false;
    samplingDone = false;
}

void Adc::stopSampling()
{
    currentSampling = NONE;
    switch (currentChannel)
    {
        case 0:
            moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_0, /*continuous=*/false);
            break;

        case 1:
            moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_1, /*continuous=*/false);
            break;

        case 2:
            moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_2, /*continuous=*/false);
            break;

        case 3:
            moduleADC.startADCReading(ADS1X15_REG_CONFIG_MUX_SINGLE_3, /*continuous=*/false);
            break;
        
        default:
            break;
    }
}