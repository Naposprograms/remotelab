#include <lab.h>

// global adc object
Adc adc(ADC_ALERT_PIN, ADC_CALIBRATION_ON_PIN);

COSPHI0;
COSPHI1;
COSPHI2;


Lab::Lab() {} // nothing to do in constructor


Lab::VoltageNode::VoltageNode(uint8_t adcChannel, bool isSource, bool isCurrent, uint8_t numberOfValues)
{
    channel = adcChannel;
    nodeIsSource = isSource;
    valueIsCurrent = isCurrent;
    valueArraySize = numberOfValues;
}


bool Lab::begin(TwoWire * usedWire)
{
    bool working = adc.begin(usedWire); 
    return working;
}


void Lab::calibrateADC()
{
    calibratingADC = true;
    busy = true;
    adc.calibrateDCOffset();
}


double truncFloat3(double value)
{
    return (int)(value * 1000 + 0.5) / 1000.0;
}


float calculateRMSValue(float * values, uint8_t numberOfValues)
{
    float squareSum = 0.0;
    if(numberOfValues > 1)
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


bool Lab::task()
{
    if(busy)
    {
        if(!waitingRelays)
        {
            if(calibratingADC)
            {
                if(adc.task())
                {
                    calibratingADC = false;
                    busy = false;
                }
            }
            else // if it's not busy for calibration, then it's busy doing a lab
            {
                if(commandSampling)
                {
                    if(missingMeassures)
                    {
                        if(missingMeassures < 4) // the adc channels are done, go on with cosphi
                        {
                            #ifdef LAB_TYPE_S
                                const char * labConfig = labResults["circuitconfig"];
                                char requestedConfig[9];
                                for(uint8_t i = 0; i < 8; i++)
                                {
                                    requestedConfig[i] = * labConfig;
                                    labConfig++;
                                }
                                requestedConfig[8] = '\0';
                                char byteToSend[8];
                                for(uint8_t i = 0; i < 8; i++)
                                {
                                    byteToSend[i] = requestedConfig[i];
                                }


                            #endif
                            commandSampling = false;
                            switch (missingMeassures)
                            {
                                case 1:
                                    cosphiMeassures[1] = cosphi1.getCosPhi();
                                    cosphiInductive[1] = cosphi1.meassuredInductive;
                                    #ifdef LAB_TYPE_S
                                        if(noMeassure[0])
                                        {
                                            cosphi0.setCosPhiValue(0, true);
                                        }
                                        else
                                        {
                                            // for meassuring cosphi0, cosphi1 and cosphi2 are shorted.
                                            if(byteToSend[forbiddenConfigSwitches[0]-1] = '0') // double check 
                                            {
                                                byteToSend[forbiddenConfigSwitches[1]-1] = '1';
                                                byteToSend[forbiddenConfigSwitches[2]-1] = '1';
                                                byteEncoder.setOutputByUInt(0, true); // powers off relays
                                                delay(50);
                                                byteEncoder.setOutputByChar(&byteToSend[0], true);
                                                waitingRelays = true;
                                                relaysTimer.set(WAIT_TIME_AFTER_SWITCHING_RELAYS);
                                                cosphi0.commandSampling(WAIT_TIME_AFTER_SWITCHING_RELAYS);
                                            }
                                            else
                                            {
                                                cosphi0.setCosPhiValue(0, true);
                                            }
                                        }
                                        #else
                                        #ifdef LAB_TYPE_P
                                        noMeassure[0] ? cosphi0.setCosPhiValue(5000, false) : cosphi0.commandSampling(0);
                                        #endif
                                    #endif
                                    break;
                                
                                case 2:
                                    cosphiMeassures[2] = cosphi2.getCosPhi();
                                    cosphiInductive[2] = cosphi2.meassuredInductive;
                                    #ifdef LAB_TYPE_S
                                        if(noMeassure[1])
                                        {
                                            cosphi1.setCosPhiValue(0, true);
                                        }
                                        else
                                        {
                                            // for meassuring cosphi1, cosphi0 and cosphi2 are shorted.
                                            if(byteToSend[forbiddenConfigSwitches[1]-1] = '0') // double check 
                                            {
                                                byteToSend[forbiddenConfigSwitches[0]-1] = '1';
                                                byteToSend[forbiddenConfigSwitches[2]-1] = '1';
                                                byteEncoder.setOutputByUInt(0, true); // powers off relays
                                                delay(50);
                                                byteEncoder.setOutputByChar(&byteToSend[0], true);
                                                waitingRelays = true;
                                                relaysTimer.set(WAIT_TIME_AFTER_SWITCHING_RELAYS);
                                                cosphi1.commandSampling(WAIT_TIME_AFTER_SWITCHING_RELAYS);
                                            }
                                            else
                                            {
                                                cosphi1.setCosPhiValue(0, true);
                                            }
                                        }
                                        #else
                                        #ifdef LAB_TYPE_P
                                        noMeassure[1] ? cosphi1.setCosPhiValue(5000, false) : cosphi1.commandSampling(0);
                                        #endif
                                    #endif
                                    break;

                                case 3:
                                    #ifdef LAB_TYPE_S
                                        if(noMeassure[2])
                                        {
                                            cosphi2.setCosPhiValue(0, true);
                                        }
                                        else
                                        {
                                            // for meassuring cosphi2, cosphi0 and cosphi1 are shorted.
                                            if(byteToSend[forbiddenConfigSwitches[2]-1] = '0') // double check 
                                            {                                            
                                                byteToSend[forbiddenConfigSwitches[0]-1] = '1';
                                                byteToSend[forbiddenConfigSwitches[1]-1] = '1';
                                                byteEncoder.setOutputByUInt(0, true); // powers off relays
                                                delay(50);
                                                byteEncoder.setOutputByChar(&byteToSend[0], true);
                                                waitingRelays = true;
                                                relaysTimer.set(WAIT_TIME_AFTER_SWITCHING_RELAYS);
                                                cosphi2.commandSampling(WAIT_TIME_AFTER_SWITCHING_RELAYS);
                                            }
                                            else
                                            {
                                                cosphi2.setCosPhiValue(0, true);
                                            }
                                        }
                                        #else
                                        #ifdef LAB_TYPE_P
                                        noMeassure[2] ? cosphi2.setCosPhiValue(5000, false) : cosphi2.commandSampling(0);
                                        #endif
                                    #endif
                                    break;
                                
                                default:
                                    break;
                            }
                        }
                        else
                        {
                            commandSampling = false;
                            adc.commandSampling(meassuresDone, SAMPLES_ARRAY_SIZE, SINGLE_CHANNEL_SAMPLING);
                        }
                    }
                }
                else
                {
                    if(missingMeassures < 4) // the adc channels are done, go on with cosphi
                    {
                        switch (missingMeassures)
                        {
                            case 1:
                                valueAvailable = cosphi0.task();
                                break;
                            
                            case 2:
                                valueAvailable = cosphi1.task();
                                break;

                            case 3:
                                valueAvailable = cosphi2.task();
                                break;
                            
                            default:
                                break;
                        }
                    }
                    else
                    {
                        valueAvailable = adc.task();
                    }

                    if(valueAvailable)
                    {
                        Serial.printf("Got meassure: %d\n", meassuresDone);
                        commandSampling = true;
                        meassuresDone++;
                        missingMeassures--;
                    }
                    if(meassuresDone >= 7)
                    {
                        cosphiMeassures[0] = cosphi0.getCosPhi();
                        cosphiInductive[0] = cosphi0.meassuredInductive;
                        busy = false;
                    }
                }
            }
        }
        else
        {
            waitingRelays = !relaysTimer.elapsed();
        }
    }
    return !busy;
}


bool Lab::doLab(const char * circuitConfig)
{
    if(busy)
    {
        return false;
    }
    else
    {
        char requestedConfig[9];
        for(uint8_t i = 0; i < 8; i++)
        {
            requestedConfig[i] = * circuitConfig;
            if(requestedConfig[i] != '0')
            {
                if(requestedConfig[i] != '1')
                {
                    requestedConfig[i] = '0'; // turns to 0 all possible values which are not 0 or 1.
                }
            }
            circuitConfig++;
        }
        requestedConfig[8] = '\0';

        bool validConfig = true;
        errorMsg = false;
        #ifdef LAB_TYPE_S
            char forbiddenValue = '1';
        #else
            #ifdef LAB_TYPE_P
                char forbiddenValue = '0';
            #endif
        #endif
        uint8_t wrongSwitches = 0;
        for(uint8_t i = 0; i < 3; i++)
        {
            noMeassure[i] = false;
            if(requestedConfig[forbiddenConfigSwitches[i]-1] == forbiddenValue)
            {
                noMeassure[i] = true;
                wrongSwitches++;
            }
        }
        if(wrongSwitches == 3)
        {
            validConfig = false;
        }

        if(validConfig)
        {
            labResults.clear(); // erases the previous JSON 
            #ifdef LAB_TYPE_S
                labResults["lab"] = "series";
            #else
                #ifdef LAB_TYPE_P
                    labResults["lab"] = "parallel";
                #endif
            #endif
            labResults["circuitconfig"] = requestedConfig;
            char byteToSend[8];
            for(uint8_t i = 0; i < 8; i++)
            {
                byteToSend[i] = requestedConfig[i];
            }
            byteEncoder.setOutputByChar(&byteToSend[0], true);
            waitingRelays = true;
            relaysTimer.set(WAIT_TIME_AFTER_SWITCHING_RELAYS);

            missingMeassures = 7; // 4 adc channels + 3 cosphi
            meassuresDone = 0;
            busy = true;
            commandSampling = true;
            return true;
        }
        else
        {
            labResults.clear(); // erases the previous JSON 
            #ifdef LAB_TYPE_S
                labResults["lab"] = "series";
                labResults["error"] = "shortcircuit";
            #else
                #ifdef LAB_TYPE_P
                    labResults["lab"] = "parallel";
                    labResults["error"] = "opencircuit";
                #endif
            #endif
            labResults["circuitconfig"] = requestedConfig;
            errorMsg = true;
            return true;
        }
    }
}


DynamicJsonDocument * Lab::getLabResults(bool fullOutput, bool labIsCorrect)
{
    if(!errorMsg)
    {
        float values[SAMPLES_ARRAY_SIZE];
        int16_t * samplesBuffer;
        if(labIsCorrect)
        {
            const char * labConfig = labResults["circuitconfig"];
            char requestedConfig[9];
            for(uint8_t i = 0; i < 8; i++)
            {
                requestedConfig[i] = * labConfig;
                labConfig++;
            }
            requestedConfig[8] = '\0';
            #ifdef LAB_TYPE_S
                char forbiddenValue = '1';
            #else
                #ifdef LAB_TYPE_P
                    char forbiddenValue = '0';
                #endif
            #endif

            uint8_t multiplier = 1;
            if(requestedConfig[0] == '1')
            {
                multiplier++;
            }
            #ifdef LAB_TYPE_S
                uint8_t closestToZeroIndexes[3] = {0, 0, 0};
                float closestToZeroValues[3] = {100.0, 100.0, 100.0};
                float voltageMeassures[SAMPLES_ARRAY_SIZE][3];
            #endif

            for(uint8_t j = 0; j < 4; j++)
            {
                #ifdef LAB_TYPE_S
                    switch (j)
                    {
                        #define NODES                                   \
                            X("voltage_A_values", "voltage_A_RMS", 0)   \
                            X("voltage_B_values", "voltage_B_RMS", 1)   \
                            X("voltage_C_values", "voltage_C_RMS", 2)

                        #define X(letterValues, letterRMS, number)                                      \
                            case number:                                                                \
                                samplesBuffer = adc.getLastChannelSamples(number, false);               \
                                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++) {                       \
                                    values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer,   \
                                    VOLTAGE_DIVIDER_FACTOR, number) * multiplier; samplesBuffer++;      \
                                    if(values[i] >= 0.0 && values[i] < closestToZeroValues[number]) {   \
                                    closestToZeroIndexes[number] = i; closestToZeroValues[number] =     \
                                    values[i]; } voltageMeassures[i][number] = values[i];               \
                                    if(fullOutput) { labResults[letterValues][i] =                      \
                                    truncFloat3(values[i]); } }                                         \
                                labResults[letterRMS] = truncFloat3(calculateRMSValue(                  \
                                &values[0], SAMPLES_ARRAY_SIZE));                                       \
                                break;
                            NODES
                        #undef X
                        
                        case 3:
                            samplesBuffer = adc.getLastChannelSamples(3, false);
                            for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                            {
                                values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, CURRENT_AMPLIFIER_FACTOR, 3);
                                samplesBuffer++;
                                if(fullOutput)
                                {
                                    labResults["current_values"][i] = truncFloat3(values[i]);
                                }
                            }
                            labResults["current_RMS"] = truncFloat3(calculateRMSValue(&values[0], SAMPLES_ARRAY_SIZE));
                            break;

                        default:
                            break;
                    }
                    /*
                    float tempValues[3];
                    tempValues[0] = labResults["voltage_A_RMS"];
                    tempValues[1] = labResults["voltage_B_RMS"];
                    tempValues[2] = labResults["voltage_C_RMS"];

                    // if relay 1 connected to GND the V drops are as meassured
                    // else if relay 1 connected to the transformer do:

                    // drops ok for NEG = transformer (in resistive)
                    labResults["voltage_AB_RMS"] = tempValues[0] - tempValues[1];
                    labResults["voltage_BC_RMS"] = 2 * tempValues[0] - (tempValues[0] - tempValues[1]) - (2 * tempValues[2]);
                    labResults["voltage_CNEG_RMS"] = 2 * tempValues[2];
                    */


                #else
                    #ifdef LAB_TYPE_P
                        switch (j)
                        {
                            #define BRANCHES                                \
                                X("current_A_values", "current_A_RMS", 0)   \
                                X("current_B_values", "current_B_RMS", 1)   \
                                X("current_C_values", "current_C_RMS", 2)

                            #define X(letterValues, letterRMS, number)                                      \
                                case number:                                                                \
                                    samplesBuffer = adc.getLastChannelSamples(number, false);               \
                                    for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++) {                       \
                                        values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer,   \
                                        CURRENT_AMPLIFIER_FACTOR, number); samplesBuffer++;                 \
                                        if(fullOutput) { if(!noMeassure[number]) {                          \
                                            labResults[letterValues][i] = truncFloat3(values[i]); }         \
                                        else { labResults[letterValues][i] = (double) 0.000; } } }          \
                                    if(!noMeassure[number]) { labResults[letterRMS] =                       \
                                    truncFloat3(calculateRMSValue(&values[0], SAMPLES_ARRAY_SIZE)); }       \
                                    else { labResults[letterRMS] = (double) 0.000; }                        \
                                    break;
                                BRANCHES
                            #undef X
                            
                            case 3:
                                samplesBuffer = adc.getLastChannelSamples(3, false);
                                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                                {
                                    values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, VOLTAGE_DIVIDER_FACTOR, 3) * multiplier;
                                    samplesBuffer++;
                                    if(fullOutput)
                                    {
                                        labResults["voltage_values"][i] = truncFloat3(values[i]);
                                    }
                                }
                                labResults["voltage_RMS"] = truncFloat3(calculateRMSValue(&values[0], SAMPLES_ARRAY_SIZE));
                                break;

                            default:
                                break;
                        }

                    #endif
                #endif

            }

            String cosphiValue = "";

            #ifdef LAB_TYPE_S

                /*
                float voltageDrops[SAMPLES_ARRAY_SIZE][3];
                uint8_t startIndex = closestToZeroIndexes[2];
                uint8_t currentIndex = startIndex;
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    if(startIndex + i == SAMPLES_ARRAY_SIZE)
                    {
                        currentIndex = 0;
                    }
                    voltageDrops[i][2] = voltageMeassures[currentIndex][2];
                    currentIndex++;
                }
                
                startIndex = closestToZeroIndexes[1];
                currentIndex = startIndex;
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    if(startIndex + i == SAMPLES_ARRAY_SIZE)
                    {
                        currentIndex = 0;
                    }
                    voltageDrops[i][1] = voltageMeassures[currentIndex][1] - voltageDrops[i][2];
                    labResults["voltage_BC_values"][i] = voltageDrops[i][1];
                    currentIndex++;
                }
                
                startIndex = closestToZeroIndexes[0];
                currentIndex = startIndex;
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    if(startIndex + i == SAMPLES_ARRAY_SIZE)
                    {
                        currentIndex = 0;
                    }
                    voltageDrops[i][0] = voltageMeassures[currentIndex][0] - voltageDrops[i][1];
                    labResults["voltage_AB_values"][i] = voltageDrops[i][0];
                    currentIndex++;
                }
                
                labResults["voltage_A_RMS"] = truncFloat3(calculateRMSValue(&voltageDrops[0][0], SAMPLES_ARRAY_SIZE));
                labResults["voltage_B_RMS"] = truncFloat3(calculateRMSValue(&voltageDrops[0][1], SAMPLES_ARRAY_SIZE));

                */
                float tempValues[3];
                tempValues[0] = labResults["voltage_A_RMS"];
                tempValues[1] = labResults["voltage_B_RMS"];
                tempValues[2] = labResults["voltage_C_RMS"];

                // if relay 1 connected to GND the V drops are as meassured
                // else if relay 1 connected to the transformer do:

                // drops ok for NEG = transformer (in resistive)
                labResults["voltage_A_RMS"] = abs(tempValues[0] - tempValues[1]);
                labResults["voltage_B_RMS"] = abs(tempValues[0] - (tempValues[0] - tempValues[1]) - tempValues[2]);

                
                cosphiValue = truncFloat3(cosphiMeassures[0]);
                cosphiInductive[0] ? cosphiValue.concat("_ind") : cosphiValue.concat("_cap");
                labResults["node_A_cosphi"] = cosphiValue;
                cosphiValue = "";

                cosphiValue = truncFloat3(cosphiMeassures[1]);
                cosphiInductive[1] ? cosphiValue.concat("_ind") : cosphiValue.concat("_cap");
                labResults["node_B_cosphi"] = cosphiValue;
                cosphiValue = "";

                cosphiValue = truncFloat3(cosphiMeassures[2]);
                cosphiInductive[2] ? cosphiValue.concat("_ind") : cosphiValue.concat("_cap");
                labResults["node_C_cosphi"] = cosphiValue;
                cosphiValue = "";
                
                #else
                #ifdef LAB_TYPE_P

                    cosphiValue = truncFloat3(cosphiMeassures[0]);
                    cosphiInductive[0] ? cosphiValue.concat("_ind") : cosphiValue.concat("_cap");
                    labResults["branch_A_cosphi"] = cosphiValue;
                    cosphiValue = "";

                    cosphiValue = truncFloat3(cosphiMeassures[1]);
                    cosphiInductive[1] ? cosphiValue.concat("_ind") : cosphiValue.concat("_cap");
                    labResults["branch_B_cosphi"] = cosphiValue;
                    cosphiValue = "";

                    cosphiValue = truncFloat3(cosphiMeassures[2]);
                    cosphiInductive[2] ? cosphiValue.concat("_ind") : cosphiValue.concat("_cap");
                    labResults["branch_C_cosphi"] = cosphiValue;
                    cosphiValue = "";

                #endif
            #endif

        }
        byteEncoder.setOutputByUInt(0, true); // powers off relays at the end of the lab.
    }

    return &labResults;
}


bool Lab::enableRelays()
{
    byteEncoder.setOutputByUInt(0, true); // to prevent relay triggering at power-on.
    byteEncoder.setOutputByUInt(0, true); // twice: one for shift and another for storage.
    byteEncoder.enableOutput(true); 
    return true;
}


float * Lab::getADCDCOffsetVolts()
{
    float * ptr = adc.getChannelsDCOffsetVolts();
    for(uint8_t ch = 0; ch < 4; ch++)
    {
        ADCchannelsOffsetVolts[ch] = * ptr;
        ptr++;
    }
    return &ADCchannelsOffsetVolts[0];
}

void Lab::writeErrorMsgToJSON(const char * errorMessage)
{
    labResults.clear();
    labResults["error"] = errorMessage;
}


bool Lab::checkBusy()
{
    return busy;
}