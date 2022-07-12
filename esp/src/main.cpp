#include <Arduino.h>
#include <config.h>
#include <led.h>
#include <adc.h>
#include <button.h>


// Global objects
AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
//SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Adc adc;
Button sampleButton(PUSH_BUTTON_PIN, false);


bool sampling = false;


float bitsToVoltage(uint16_t bits)
{
    return 36 * ((AC_ZERO_VOLTS * (bits) / AC_ZERO_BITS) - AC_ZERO_VOLTS);
}


float rmsValue(float * values, uint8_t numberOfValues)
{
    float square = 0.0;
    if(numberOfValues > 0)
    {
        for (uint8_t i = 0; i < numberOfValues; i++)
        {
            square += pow(* values, 2);
            values++;
        }
        return sqrt(square / numberOfValues);
    }
    else
    {
        return * values;
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    bool adcModuleWorking = adc.initiate();
    if(adcModuleWorking)
    {
        Serial.println("ADC module OK");
    }
    else
    {
        Serial.println("ADC module NOT WORKING");
    }
}


void loop()
{
    ledAlive.taskAliveLed();
    //ledSignal.taskSignalLed();

    if(sampleButton.pressed())
    {
        adc.commandSampling(0, true);
        sampling = true;
    }
    if(sampling)
    {
        if(adc.task())
        {
            int16_t * samplesBuffer = adc.getLastChannelSamples(0);
            Serial.println("SAMPLES");
            float samplesArray[SAMPLES_ARRAY_SIZE];
            float voltage, voltageRMS;
            for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
            {
                voltage = bitsToVoltage(* samplesBuffer);
                samplesArray[i] = voltage;
                Serial.printf("S#%d:%f\n", i, voltage);
                samplesBuffer++;          
            }
            voltageRMS = rmsValue(&samplesArray[0], SAMPLES_ARRAY_SIZE);
            Serial.printf("RMS voltage: %f\n", voltageRMS);
            
            sampling = false;
            adc.stopSampling();
            sampleButton.release();
        }
    }
}