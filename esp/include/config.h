#ifndef CONFIG_H
#define CONFIG_H

#define ESP32 1

// LAB
//#define LAB_TYPE_S 's'
#ifndef LAB_TYPE_S
    #define LAB_TYPE_P 'p'
#endif // = 's' for series, 'p' for parallel


// PINS
#define BAUD_RATE 115200
#define PUSH_BUTTON_PIN 0
#define LED_ALIVE_PIN 2
#define LED_SIGNAL_PIN 23
#define ADC_ALERT_PIN 19
#define ADC_CALIBRATION_ON_PIN 18
#define SHIFTER_LATCH_PIN 25
#define SHIFTER_CLOCK_PIN 26
#define SHIFTER_DATA_PIN 27
#define SHIFTER_ENABLE_PIN 33

#ifdef LAB_TYPE_S
    #define CURRENT_ZERO_CROSS_SIGNAL_PIN 17
    #define VOLTAGE_0_ZERO_CROSS_SIGNAL_PIN 16
    #define VOLTAGE_1_ZERO_CROSS_SIGNAL_PIN 4
    #define VOLTAGE_2_ZERO_CROSS_SIGNAL_PIN 15
#else
    #ifdef LAB_TYPE_P
        #define VOLTAGE_ZERO_CROSS_SIGNAL_PIN 17
        #define CURRENT_0_ZERO_CROSS_SIGNAL_PIN 16
        #define CURRENT_1_ZERO_CROSS_SIGNAL_PIN 4
        #define CURRENT_2_ZERO_CROSS_SIGNAL_PIN 15
    #endif
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
#define CURRENT_AMPLIFIER_FACTOR 0.1 // the amplifier has gain x10 so the measure should be shrinked by 0.1
#define TIME_BETWEEN_CALIBRATIONS_MS 3 * 60 * 1000 // 15 minutes in ms

#define WAIT_TIME_AFTER_SWITCHING_RELAYS 500

#endif