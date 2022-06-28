#include <Arduino.h>
#include <config.h>
#include <led.h>
#include <adc.h>

// Global objects
AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
//SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Adc adc;


bool samplesReady, findingZero, buttonPushed;
float lastValue, formerLastValue;


void triggerZeroFinding()
{
    samplesReady = false;
    findingZero = true;
    formerLastValue = -1;
    lastValue = -1;
}

bool detectRisingZeroCross(int16_t newValue)
{
    bool crossed = false;
    if(lastValue < 0 && formerLastValue < 0)
    {
        if(lastValue < 0 && newValue >= 0)
        {
            crossed = true;
        }
    }
    formerLastValue = lastValue;
    lastValue = newValue;
    return crossed;
}

float bitsToVoltage(uint16_t bits)
{
    return 36 * ((AC_ZERO_VOLTS * (bits) / AC_ZERO_BITS) - AC_ZERO_VOLTS);
}


float rmsValue(float * values, uint8_t numberOfValues)
{
    float square = 0.0; 
    for (uint8_t i = 0; i < numberOfValues; i++)
    {
        square += pow(* values, 2);
        values++;
    }
    return sqrt(square / numberOfValues);
}

void setup()
{
    Serial.begin(BAUD_RATE);
    pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);
    bool adcModuleWorking = adc.initiate();
    if(adcModuleWorking)
    {
        Serial.println("ADC module OK");
        adc.commandSampling(0, false);
        triggerZeroFinding();
    }
    else
    {
        Serial.println("ADC module NOT WORKING");
    }
    buttonPushed = false;
}


void loop()
{
    ledAlive.taskAliveLed();
    //ledSignal.taskSignalLed();
    samplesReady = adc.task();

    if(!buttonPushed)
    {
        if(digitalRead(PUSH_BUTTON_PIN) == LOW)
        {
            buttonPushed = true;
            adc.commandSampling(0, false);
            triggerZeroFinding();
            Serial.println("Button pushed: Triggered zero finding");
        }
    }

    if(samplesReady)
    {
        if(buttonPushed)
        {
            if(findingZero)
            {
                int16_t * sample = adc.getLastChannelSamples(0);
                bool zeroFound = detectRisingZeroCross(bitsToVoltage(* sample));
                if(zeroFound)
                {
                    Serial.println(bitsToVoltage((*sample)));
                    adc.commandSampling(0, true);
                    findingZero = false;
                    Serial.println("Zero crossed: now sampling...");
                }
                else
                {
                    adc.commandSampling(0, false);
                }
            }
            else
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
                adc.stopSampling();
                buttonPushed = false; // we can now push again, to command another sampling
            }
        }
    }
}