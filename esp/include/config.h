// uncomment this line to adapt code for ESP32. Leave commented for arduino uno code. 
#define ESP32 1

// PINS
#ifndef ESP32
    #define BAUD_RATE 9600
    #define PUSH_BUTTON_PIN 4
    #define LED_ALIVE_PIN 13
    #define LED_SIGNAL_PIN 5
    #define ADC_ALERT_PIN 3
#else
    #define BAUD_RATE 115200
    #define PUSH_BUTTON_PIN 0
    #define LED_ALIVE_PIN 2
    #define LED_SIGNAL_PIN 23
    #define ADC_ALERT_PIN 19
    #define ADC_CALIBRATION_ON_PIN 18
    #define VOLTAGE_ZERO_CROSS_SIGNAL_PIN 17
    #define CURRENT_ZERO_CROSS_SIGNAL_PIN 16    
#endif


// LEDS
#define LED_ALIVE_TIME_ON 150
#define LED_ALIVE_TIME_OFF 850
#define LED_SIGNAL_TIME_ON 200
#define LED_SIGNAL_TIME_OFF 400

// ADC
// samples per second = 860 for ADS1115
// for f = 50 Hz, T = 20 ms (there will be 17 samples of each period)
#define SAMPLES_PER_PERIOD 17
#define PERIODS_TO_SAMPLE 5
#define SAMPLES_ARRAY_SIZE (SAMPLES_PER_PERIOD * PERIODS_TO_SAMPLE)
#define VOLTAGE_DIVIDER_FACTOR 33.35 
#define CURRENT_AMPLIFIER_FACTOR 0.1 // the amplifier has gain x10 so it has to be shrinked by 0.1
