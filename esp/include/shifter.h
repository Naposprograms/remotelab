#ifndef SHIFTER_H
#define SHIFTER_H


/**
 * @brief Simple shifter object to receive a byte and serialize it into 74HC595 IC.
 */


#include <Arduino.h>


class Shifter {

    private:

        uint8_t latch, clock, data, enable;
        uint8_t byteStrToInt(const char * byteStr);


    public:
        /**
         * @brief Construct a new Shifter object
         * 
         */
        Shifter(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t enable);
        void enableOutput(bool enableOutput);
        void setOutputByUInt(uint8_t outputCode, bool invertLogic);
        void setOutputByChar(const char * codeStr, bool invertLogic);


};

#endif