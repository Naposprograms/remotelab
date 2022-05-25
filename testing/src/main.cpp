#include <Arduino.h>
#include <config.h>
#include <led.h>
#include <Adafruit_ADS1X15.h>


AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Adafruit_ADS1115 moduleADC;
Timer adcTimer;


void setup()
{
    Serial.begin(9600);
    if(moduleADC.begin())
    {
        ledSignal.blink(3);
        adcTimer.set(500);
    }
}

void loop()
{
    ledAlive.taskAliveLed();
    ledSignal.taskSignalLed();
    if(adcTimer.elapsed())
    {
        int16_t adc0, adc1, adc2, adc3;
        float volts0, volts1, volts2, volts3;

        adc0 = moduleADC.readADC_SingleEnded(0);
        adc1 = moduleADC.readADC_SingleEnded(1);
        adc2 = moduleADC.readADC_SingleEnded(2);
        adc3 = moduleADC.readADC_SingleEnded(3);

        volts0 = moduleADC.computeVolts(adc0);
        volts1 = moduleADC.computeVolts(adc1);
        volts2 = moduleADC.computeVolts(adc2);
        volts3 = moduleADC.computeVolts(adc3);

        Serial.println("-----------------------------------------------------------");
        Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
        Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
        Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
        Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");


        adcTimer.set(500);
    }
}