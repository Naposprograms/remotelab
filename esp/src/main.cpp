#include <Arduino.h>
#include <config.h>
#include <led.h>
#include <adc.h>
#include <button.h>
#include <cosphi.h>


// Global objects
AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Button sampleButton(PUSH_BUTTON_PIN, ACTIVE_LOW);
Adc adc(ADC_ALERT_PIN, ADC_CALIBRATION_ON_PIN);
CosPhi cosphi(VOLTAGE_ZERO_CROSS_SIGNAL_PIN, CURRENT_ZERO_CROSS_SIGNAL_PIN);


bool sampling, calibrating, cosPhiReady = false;
uint8_t currentChannel, buttonPressedTimes = 0;
float storedVoltage, storedCurrent = 0.0;


float calculateRMSValue(float * values, uint8_t numberOfValues)
{
    float squareSum = 0.0;
    if(numberOfValues > 0)
    {
        for (uint8_t i = 0; i < numberOfValues; i++)
        {
            squareSum += pow(* values, 2);
            values++;
        }
        return sqrt(squareSum / numberOfValues);
    }
    else
    {
        return * values;
    }
}


void setup()
{
    delay(500); // delays on power ON to allow caps to charge and voltage levels to stabilize.
    Serial.begin(BAUD_RATE);
    adc.begin() ? Serial.println("ADC OK") : Serial.println("ADC NOT WORKING");
}


void loop()
{
    ledAlive.taskAliveLed();
    ledSignal.taskSignalLed();

    if(sampleButton.pressed())
    {
        ledSignal.blink(1);
        if(!buttonPressedTimes)
        {
            adc.calibrateDCOffset();
            calibrating = true;
        }
        else
        {
            adc.commandSampling(currentChannel, SAMPLES_ARRAY_SIZE, SINGLE_CHANNEL_SAMPLING);
            //adc.commandSampling(currentChannel, SAMPLES_ARRAY_SIZE, DIFFERENTIAL_SAMPLING);
            sampling = true;
        }
        buttonPressedTimes++;
    }

    if(calibrating)
    {
        if(adc.task())
        {
            Serial.println("ADC DC offset calibrated.");
            calibrating = false;
            sampleButton.release();
        }
    }

    if(sampling)
    {
        if(adc.task())
        {
            int16_t * samplesBuffer = adc.getLastChannelSamples(currentChannel, SINGLE_CHANNEL_SAMPLING);
            Serial.printf("CH%d - ", currentChannel);
            float samplesArray[SAMPLES_ARRAY_SIZE];
            float voltage, voltageRMS;
            if(!currentChannel) // it's CH0
            {
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    voltage = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, VOLTAGE_DIVIDER_FACTOR, currentChannel);
                    samplesArray[i] = voltage;
                    samplesBuffer++;          
                }
                voltageRMS = calculateRMSValue(&samplesArray[0], SAMPLES_ARRAY_SIZE);
                Serial.printf("Voltage: %f\n", voltageRMS);
                storedVoltage = voltageRMS;
                currentChannel += 2;
                sampling = false;
                sampleButton.release();
            }
            else // it's ch2 
            {
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    voltage = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, CURRENT_AMPLIFIER_FACTOR, currentChannel);
                    samplesArray[i] = voltage;
                    samplesBuffer++;          
                }
                voltageRMS = calculateRMSValue(&samplesArray[0], SAMPLES_ARRAY_SIZE);
                Serial.printf("Current: %f\n", voltageRMS);
                storedCurrent = voltageRMS;
                Serial.printf("Aparent Power (S): %f\n", storedCurrent * storedVoltage);
                currentChannel = 0;
                cosphi.commandSampling();
                sampling = false;
                sampleButton.release();
            }
        }
    }

    if(cosphi.task())
    {
        cosPhiReady = true;
    }
    if(cosPhiReady)
    {
        cosPhiReady = false;
        float cosPhiValue = cosphi.getCosPhi();
        Serial.printf("Cos Phi: %f\n", cosPhiValue);
        Serial.printf("Active Power (P): %f\n", storedCurrent * storedVoltage * cosPhiValue);
    }
}