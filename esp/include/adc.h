#include <Arduino.h>
#include <config.h>
#include <Adafruit_ADS1X15.h>
// source: https://github.com/adafruit/Adafruit_ADS1X15
// source: https://github.com/adafruit/Adafruit_BusIO


// To learn about X macros go to https://www.geeksforgeeks.org/x-macros-in-c/
// X macros examples at https://stackoverflow.com/questions/6635851/real-world-use-of-x-macros
#define CHANNELS    \
    X(ch0, 0)       \
    X(ch1, 1)       \
    X(ch2, 2)       \
    X(ch3, 3)       


struct channelSamples
{
    int16_t singleSample;
    int16_t samplesBuffer[SAMPLES_ARRAY_SIZE];
    int16_t * bufferPtr = &samplesBuffer[0];
};

enum samplingRequest
{
    NONE,
    SINGLE_SAMPLE,
    MANY_SAMPLES,
};


class Adc {

    private:

        Adafruit_ADS1115 moduleADC;
        bool samplingDone;
        uint8_t missingSamples, currentChannel;
        int16_t nullSampleValue = 0;
        int16_t * nullSample = &nullSampleValue;

        #define X(varName, chNumber) channelSamples varName;
            CHANNELS
        #undef X

        samplingRequest currentSampling;

        void routeSample();

    public:

        Adc();
        ~Adc();
        bool initiate();
        bool task();

        int16_t * getLastChannelSamples(uint8_t channel);
        void commandSampling(uint8_t channel, bool many);
        void stopSampling();

};