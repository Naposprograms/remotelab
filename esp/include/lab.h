#ifndef LAB_H
#define LAB_H


#include <Arduino.h>
#include <config.h>
#include <adc.h>
#include <cosphi.h>
#include <ArduinoJson.h>


#define KILOBYTE 1024
static DynamicJsonDocument labResults((unsigned long) 10 * KILOBYTE);


class Lab {

    private:
        // c reallocation https://www.geeksforgeeks.org/dynamic-memory-allocation-in-c-using-malloc-calloc-free-and-realloc/
        
        bool busy, calibratingADC, commandSampling, valueAvailable = false;
        const char * forbiddenConfig = "00011100";
        uint8_t missingMeassures, meassuresDone = 0;


        class VoltageNode {

            private:

                bool nodeIsSource, valueIsCurrent;
                uint8_t channel, valueArraySize;
            
            public:

                VoltageNode(uint8_t adcChannel, bool isSource, bool isCurrent, uint8_t numberOfValues);
                
                float * getValues();
                //float calculateRMSValue(float * values, uint8_t numberOfValues);

        };

        #ifdef LAB_TYPE_S
            char circuit = 's'; // = 's' for series
            #define ADC0 VoltageNode adc0(0, true, false, SAMPLES_ARRAY_SIZE)
            #define ADC1 VoltageNode adc1(1, false, false, SAMPLES_ARRAY_SIZE);
            #define ADC2 VoltageNode adc2(2, false, false, SAMPLES_ARRAY_SIZE);
            #define ADC3 VoltageNode adc3(3, false, true, SAMPLES_ARRAY_SIZE);
            #define COSPHI0 CosPhi cosphi0(VOLTAGE_0_ZERO_CROSS_SIGNAL_PIN, CURRENT_ZERO_CROSS_SIGNAL_PIN);
            #define COSPHI1 CosPhi cosphi1(VOLTAGE_1_ZERO_CROSS_SIGNAL_PIN, CURRENT_ZERO_CROSS_SIGNAL_PIN);
            #define COSPHI2 CosPhi cosphi2(VOLTAGE_2_ZERO_CROSS_SIGNAL_PIN, CURRENT_ZERO_CROSS_SIGNAL_PIN);
        #else
            #ifdef LAB_TYPE_P
                char circuit = 'p'; // = 'p' for parallel
                #define ADC0 VoltageNode adc0(0, false, true, SAMPLES_ARRAY_SIZE);
                #define ADC1 VoltageNode adc1(1, false, true, SAMPLES_ARRAY_SIZE);
                #define ADC2 VoltageNode adc2(2, false, true, SAMPLES_ARRAY_SIZE);
                #define ADC3 VoltageNode adc3(3, true, false, SAMPLES_ARRAY_SIZE);
                /*
                #define ADC0 adc0(0, false, true, SAMPLES_ARRAY_SIZE);
                #define ADC1 adc1(1, false, true, SAMPLES_ARRAY_SIZE);
                #define ADC2 adc2(2, false, true, SAMPLES_ARRAY_SIZE);
                #define ADC3 adc3(3, true, false, SAMPLES_ARRAY_SIZE);
                */
                #define COSPHI0 CosPhi cosphi0(VOLTAGE_ZERO_CROSS_SIGNAL_PIN, CURRENT_0_ZERO_CROSS_SIGNAL_PIN);
                #define COSPHI1 CosPhi cosphi1(VOLTAGE_ZERO_CROSS_SIGNAL_PIN, CURRENT_1_ZERO_CROSS_SIGNAL_PIN);
                #define COSPHI2 CosPhi cosphi2(VOLTAGE_ZERO_CROSS_SIGNAL_PIN, CURRENT_2_ZERO_CROSS_SIGNAL_PIN);
            #endif
        #endif
        /*
        ADC0;
        ADC1;
        ADC2;
        ADC3;
        */
    

    public:

        Lab();

        /**
         * @brief Call in main to run the lab process
         * 
         * @return true if there are results ready
         * @return false if there are no results ready
         */
        bool task();

        /**
         * @brief Comands to run all measures with an specific circuit configuration
         * 
         * @param circuitConfig The byte with the circuit configuration
         * @return true If the lab can be done
         * @return false If the lab cannot be done (busy, wrong byte code received, wrong lab type)
         */
        bool doLab(const char * circuitConfig);

        /**
         * @brief Get the Lab Results JSON object
         * 
         * @param fullOutput true if the JSON object should contain all of the samples.
         *                      false to receive only the RMS values
         * @return const char * to JSON text of the lab results
         */
        DynamicJsonDocument * getLabResults(bool fullOutput);


        const char * getErrorMsg();

        bool begin(TwoWire * usedWire);

        void calibrateADC();
        
};

#endif