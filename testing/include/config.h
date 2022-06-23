// uncomment this line to adapt code for ESP32. Leave commented for arduino code. 
//#define ESP32 1
/*
#ifndef ESP32
    #define ARDUINO 1
#endif*/

// LEDS
#define LED_ALIVE_PIN 5
#define LED_ALIVE_TIME_ON 150
#define LED_ALIVE_TIME_OFF 850
#define LED_SIGNAL_PIN 13
#define LED_SIGNAL_TIME_ON 200
#define LED_SIGNAL_TIME_OFF 400

// ADC
// samples per second = 860, sampled signal period = 20 ms (therefore 17 samples per period) 
#define SAMPLES_ARRAY_SIZE 34
#define ADC_ALERT_PIN 3
#define AC_ZERO_BITS 22300
#define AC_ZERO_VOLTS 1.46