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
bool sampling = false;
bool calibrating = false;


const char * dummyCircuitConfig = "10101010";
DynamicJsonDocument * labJSON;


void setup()
{
    delay(500); // delays on power ON to allow caps to charge and voltage levels to stabilize.
    Serial.begin(BAUD_RATE);
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
            if(calibrationTimer.elapsed())
            {
                calibrating = false;
                calibrationTimer.set(TIME_BETWEEN_CALIBRATIONS_MS);
            }
            if(sampling)
            {
                labJSON = lab.getLabResults(true);
                serializeJsonPretty(* labJSON, Serial);
                sampling = false;
                sampleButton.release();
            }
        }
    }
    if(sampleButton.pressed())
    {
        ledSignal.blink(1);
        sampling = lab.doLab(dummyCircuitConfig);
    }
}