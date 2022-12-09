#include <Arduino.h>
#include <config.h>
#include <button.h>
#include <led.h>
#include <lab.h>


// Global objects
AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Button sampleButton(PUSH_BUTTON_PIN, ACTIVE_LOW);
Lab lab;
Timer calibrationTimer;


bool adcOK = false;
bool relaysEnabled = false;
bool sampling = false;
bool calibrating = false;

bool circuitAB = false;
const char * dummyCircuitConfig_A = "10101010";
const char * dummyCircuitConfig_B = "01010101";
DynamicJsonDocument * labJSON;


void setup()
{
    delay(500); // delays on power ON to allow caps to charge and voltage levels to stabilize.
    Serial.begin(BAUD_RATE);
    relaysEnabled = lab.enableRelays();
    adcOK = lab.begin(&Wire); 
    adcOK ? Serial.println("ADC OK") : Serial.println("ADC NOT WORKING");
    if(adcOK)
    {
        lab.calibrateADC(); // must be called for the first time before any meassuring
        calibrationTimer.set(TIME_BETWEEN_CALIBRATIONS_MS);
        calibrating = true;
    }
}


void loop()
{
    ledAlive.taskAliveLed();
    ledSignal.taskSignalLed();

    if(sampling || calibrating)
    {
        if(lab.task()) // true when the lab is finished or the device is idle
        {
            if(sampling)
            {
                labJSON = lab.getLabResults(false);
                serializeJsonPretty(* labJSON, Serial);
                sampling = false;
                sampleButton.release();
            }
            if(calibrating)
            {
                float * ptr = lab.getADCDCOffsetVolts();
                for(uint8_t chNumber = 0; chNumber < 4; chNumber++)
                {
                    Serial.printf("CH%d DC offset: %f\n", chNumber, * (ptr + chNumber));
                }
                calibrating = false;
                calibrationTimer.set(TIME_BETWEEN_CALIBRATIONS_MS);
            }
        }
    }
    else
    {
        if(calibrationTimer.elapsed() && !calibrating)
        {
            lab.calibrateADC();
            calibrating = true;
        }
    }

    if(sampleButton.pressed())
    {
        ledSignal.blink(1);
        circuitAB = !circuitAB;
        if(circuitAB)
        {
            sampling = lab.doLab(dummyCircuitConfig_A);
        }
        else
        {
            sampling = lab.doLab(dummyCircuitConfig_B);
        }
    }
}