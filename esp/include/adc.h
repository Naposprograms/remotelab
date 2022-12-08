#ifndef ADC_H
#define ADC_H



// missing more testing on calibration (while using scope to compare with hardware)
// the esp32 being on disturbs the amp int output signal (and the gain set resistor signal)


/**
 * @brief This is a non-blocking module used to interact with the ADC module ADS1115.
 * It allows to command single or multiple samples, on single channels or differential between channels.
 * The task will inform when the commanded samples are ready to be read.
 */


#include <Arduino.h>
#include <timer.h>
#include <Adafruit_ADS1X15.h>
// source: https://github.com/adafruit/Adafruit_ADS1X15
// source: https://github.com/adafruit/Adafruit_BusIO


#define SINGLE_CHANNEL_SAMPLING 0
#define DIFFERENTIAL_SAMPLING 1
#define ONE_SAMPLE 0
#define MULTIPLE_SAMPLES 1

#define MAX_SAMPLES_ARRAY_SIZE 128
#define FULL_SCALE_VOLTAGE 2.048
#define FULL_SCALE_BITS 32767 // it is a int16_t type
// The real DC offset bits can be measured grounding all AC inputs.
#define DEFAULT_DC_OFFSET_BITS int(FULL_SCALE_BITS / 2)
#define DEFAULT_DC_OFFSET (FULL_SCALE_VOLTAGE / 2)


class Adc {

    private:

        Adafruit_ADS1115 moduleADC;
        Timer calibrationTimer;

        bool samplingDone, currentSamplingIsDifferential, capsDischarged;
        uint8_t missingSamples, missingChannelsCalibration, samplesArraySize;
        uint8_t currentChannel, alertPin, calibrationPin;
        int16_t nullSampleValue = 0;
        int16_t * nullSample = &nullSampleValue;
        int16_t channelsOffsetBits[4];
        float channelsOffsetVolts[4];


        // To learn about X macros go to https://www.geeksforgeeks.org/x-macros-in-c/
        // X macros examples at https://stackoverflow.com/questions/6635851/real-world-use-of-x-macros
        #define CHANNELS    \
            X(ch0, 0)       \
            X(ch1, 1)       \
            X(ch2, 2)       \
            X(ch3, 3)

        #define DIFF_CHANNELS   \
            X(chDiff03, 0)      \
            X(chDiff13, 1)      \
            X(chDiff23, 2)
        // The possibilities are - ch3 + each of the others
        // and ch0 - ch1 (numbered as 3 in adc.cpp) uses channelSamples chDiff01


        struct channelSamples
        {
            int16_t singleSample;
            int16_t samplesBuffer[MAX_SAMPLES_ARRAY_SIZE];
            int16_t * bufferPtr = &samplesBuffer[0];
            int16_t offsetBits = DEFAULT_DC_OFFSET_BITS;
        };

        enum samplingRequest
        {
            NONE,
            SINGLE_SAMPLE,
            MANY_SAMPLES,
            CALIBRATION,
        };


        #define X(varName, chNumber) channelSamples varName;
            CHANNELS
            DIFF_CHANNELS
        #undef X
        channelSamples chDiff01;
        samplingRequest currentSampling;

        void routeSample();


    public:
        /**
         * @brief Construct an ADC object.
         */
        Adc(uint8_t adcAlertPin, uint8_t calibrationSignalPin);

        /**
         * @brief This method initializes the communication with the ADC module
         * and configures the module's parameters (gain and samples per second).
         * @return true if the module is working OK.
         * @return false if something failed.
         */
        bool begin(TwoWire * usedWire);

        /**
         * @brief The task must be periodically called in the loop.
         * 
         * @return true if the commanded samples are ready to be read.
         * @return false if the commanded samples are not available yet.
         */
        bool task();

        /**
         * @brief This method commands to start a sampling process.
         * 
         * @param channel The ADC channel desired to sample (single or differential).
         * @param numberOfSamples The number of samples to command.
         * @param differential true to command a differential sampling between the specified channels.
         * (see the X-Macro DIFF_CHANNELS)
         */
        void commandSampling(uint8_t channel, uint8_t numberOfSamples, bool differential);

        /**
         * @brief Get the Last Single Sample from the specified single or differential channel. 
         * 
         * @param channel The ADC channel desired to read.
         * @param differential true to get the differential between channels, false for single channel.
         * @return int16_t 0 if no sampling has happened yet for the specified channel.
         * Otherwise it returns the number of bits according to the measured voltage.
         */
        int16_t getLastSingleSample(uint8_t channel, bool differential);

        /**
         * @brief Get the Last Channel Samples object
         * 
         * @param channel The ADC channel desired to read.
         * @param differential true to get the differential between channels, false for single channel.
         * @return int16_t* A pointer to a variable storing the number 0 if no sampling has happened yet.
         * Otherwise it returns a pointer to an array of samples with the number of bits according to the samples.
         */
        int16_t * getLastChannelSamples(uint8_t channel, bool differential);

        /**
         * @brief Triggers a signal to ground all AC inputs and commands a sampling process for each channel.
         * There is a brief non-blocking delay for capacitors to discharge before meassuring the DC offset.
         * Then it stores the meassured bits to know the real DC offset of each channel.
         */
        void calibrateDCOffset();

        /**
         * @brief Converts the adquired bits value to a voltage using a provided scaling factor.
         * 
         * @param bits The number of bits provided by the ADC.
         * @param signalAmplitudeFactor A factor to adjust the voltage value if the sample is taken from
         * an amplifier or a voltage divider which modifies the actual voltage to meassure.
         * @return float The voltage value corresponding to the meassured bits times the provided factor.
         */
        float convertBitsToVoltage(int16_t bits, float signalAmplitudeFactor);

        /**
         * @brief Converts the adquired bits value to a voltage using a provided scaling factor and
         * adjusted by the DC offset of the specified ADC channel.
         * 
         * @param bits The number of bits provided by the ADC.
         * @param signalAmplitudeFactor A factor to adjust the voltage value if the sample is taken from
         * an amplifier or a voltage divider which modifies the actual voltage to meassure.
         * @param adcChannel The number of the ADC channel from which the sample was taken, to use the provided
         * ADC channel's DC offset to adjust the meassured voltage.
         * @return float The voltage value corresponding to the meassured bits times the provided factor
         * and adjusted by the DC offset of the specified ADC channel.
         */
        float convertBitsToVoltageWithDCOffset(int16_t bits, float signalAmplitudeFactor, uint8_t adcChannel);

        /**
         * @brief Get the Channels DC Offset meassured in Bits
         * 
         * @return int16_t * pointer to ch0 bits (do ptr++ to read ch1, ch2, ch3)
         */
        int16_t * getChannelsDCOffsetBits();

        /**
         * @brief Get the Channels DC Offset in Volts
         * 
         * @return float * pointer to ch0 volts (do ptr++ to read ch1, ch2, ch3)
         */
        float * getChannelsDCOffsetVolts();
};

#endif