#include <shifter.h>



Shifter::Shifter(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t enablePin)
{
    latch = latchPin;
    clock = clockPin;
    data = dataPin;
    enable = enablePin;
    pinMode(latch, OUTPUT);
    pinMode(clock, OUTPUT);
    pinMode(data, OUTPUT);
    pinMode(enable, OUTPUT);
    digitalWrite(enable, LOW); // to be sure that 74HC595 is disabled at power-on 
}


void Shifter::enableOutput(bool enableOutput)
{
    digitalWrite(enable, enableOutput);
}

uint8_t Shifter::byteStrToInt(const char * byteStr)
{

    /*

    char* is a mutable pointer to a mutable character/string.

    const char* is a mutable pointer to an immutable character/string. You cannot change the contents of the location(s) this pointer points to. Also, compilers are required to give error messages when you try to do so. For the same reason, conversion from const char * to char* is deprecated.

    char* const is an immutable pointer (it cannot point to any other location) but the contents of location at which it points are mutable.

    const char* const is an immutable pointer to an immutable character/string.

    */

    uint8_t bitValue, signicanceBit, byteCode = 0;
    for(uint8_t i = 0; i < 8; i++)
    {
        signicanceBit = pow(2, 7 - i);
        bitValue = ((* (byteStr + i)) - '0'); // convert char to uint8_t (for numbers)
        if(bitValue == 1) // if the bit was 1, it adds the significance value
        {
            byteCode += signicanceBit;
        } // for values = 0 or others (wrong values, non-bits) there's no need to add anything
    }
    return byteCode;
}

void Shifter::setOutputByUInt(uint8_t outputCode, bool invertLogic)
{
    digitalWrite(latch, LOW);
    if(invertLogic)
    {
        outputCode = ~outputCode;
    }
    // changed MSBFIRST for LSBFIRST
    shiftOut(data, clock, LSBFIRST, outputCode);
    digitalWrite(latch, HIGH);
}

void Shifter::setOutputByChar(const char * byteStr, bool invertLogic)
{
    setOutputByUInt(Shifter::byteStrToInt(byteStr), invertLogic);
}
//https://docs.arduino.cc/tutorials/communication/guide-to-shift-out