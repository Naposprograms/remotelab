#include <ArduinoJson.h>

//globals

char webJSON[] = "{\n    \"lab\": \"parallel\",\n    \"circuitconfig\": \"01000110\"\n}";
// use one JSON object for Rx and a different one for Tx
DynamicJsonDocument docRx(1024); 
DynamicJsonDocument docTx(1024);
uint8_t byteCode = 0;
char circuitConfig[8] = {'0', '0', '0', '0', '0', '0', '0', '0'};
const char * circuitConfigReader = &circuitConfig[0];

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(250);
    Serial.println("Hello world");


    deserializeJson(docRx, webJSON);
    circuitConfigReader = docRx["circuitconfig"];
    Serial.println(circuitConfigReader);

    uint8_t bitValue, signicanceBit;
    byteCode = 0;

    for(uint8_t i = 0; i < 8; i++)
    {
        circuitConfig[i] = * (circuitConfigReader + i);
        signicanceBit = pow(2, 7 - i);
        bitValue = (circuitConfig[i] - '0'); // convert char to uint8_t (for numbers)
        Serial.printf("\nsignicantBit bit: %d\n", signicanceBit); // prints 128, 64, 32 ...
        Serial.printf("\nbitValue: %d\n", bitValue); // prints 0, 1, 0 ...
        
        // if the bit was 1, it adds the significance value
        // for values = 0 or others (wrong values, non-bits) there's no need to add anything
        if(bitValue == 1) 
        {
            byteCode += signicanceBit; // adds + 64, + 4, + 2
        } 
    }

    Serial.printf("\nbyteCode (uint): %d\n", byteCode); // for 01000110, prints 70 = 64 + 4 + 2

    if(byteCode == FORBIDDEN_NUMBER)
    {
        Serial.println("Shortcircuit/Open Circuit");
    }
    else
    {
        relays(byteCode);
    }
}
