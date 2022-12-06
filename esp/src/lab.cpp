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
                    Serial.println("ADC DC offset calibrated.");
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
                            commandSampling = false;
                            switch (missingMeassures)
                            {
                                case 1:
                                    cosphiMeassures[1] = cosphi1.getCosPhi();
                                    cosphi0.commandSampling();
                                    break;
                                
                                case 2:
                                    cosphiMeassures[2] = cosphi2.getCosPhi();
                                    cosphi1.commandSampling();
                                    break;

                                case 3:
                                    cosphi2.commandSampling();
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
                        //Serial.printf("Got meassure: %d\n", meassuresDone);
                        commandSampling = true;
                        meassuresDone++;
                        missingMeassures--;
                    }
                    if(meassuresDone >= 7)
                    {
                        cosphiMeassures[0] = cosphi0.getCosPhi();
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
            circuitConfig++;
        }
        requestedConfig[8] = '\0';
        int8_t compare = strcmp(&requestedConfig[0], Lab::forbiddenConfig);
        //Serial.printf("Strcmp: %d\n", compare);
        
        if(compare != 0)
        {
            Serial.println("Circuit config ok. Commanding samples...");
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
            // JSON with error msg
            return false;
        }
    }
}


DynamicJsonDocument * Lab::getLabResults(bool fullOutput)
{
    float tempValue;
    float values[SAMPLES_ARRAY_SIZE];
    int16_t * samplesBuffer;
    for(uint8_t j = 0; j < 4; j++)
    {
        switch (j)
        {
            // add X-macro to handle all channels and another for all cosphi.
            // then replicate inside #ifdef for series circuit config

            case 0:
                samplesBuffer = adc.getLastChannelSamples(0, false);
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, CURRENT_AMPLIFIER_FACTOR, 0);
                    samplesBuffer++;
                    if(fullOutput)
                    {
                        labResults["current_A_values"][i] = truncFloat3(values[i]);
                    }
                }
                labResults["current_A_RMS"] = truncFloat3(calculateRMSValue(&values[0], SAMPLES_ARRAY_SIZE));
                break;

            case 1:
                samplesBuffer = adc.getLastChannelSamples(1, false);
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, CURRENT_AMPLIFIER_FACTOR, 1);
                    samplesBuffer++;
                    if(fullOutput)
                    {
                        labResults["current_B_values"][i] = truncFloat3(values[i]);
                    }
                }
                labResults["current_B_RMS"] = truncFloat3(calculateRMSValue(&values[0], SAMPLES_ARRAY_SIZE));
                break;

            case 2:
                samplesBuffer = adc.getLastChannelSamples(2, false);
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, CURRENT_AMPLIFIER_FACTOR, 2);
                    samplesBuffer++;
                    if(fullOutput)
                    {
                        labResults["current_C_values"][i] = truncFloat3(values[i]);
                    }
                }
                labResults["current_C_RMS"] = truncFloat3(calculateRMSValue(&values[0], SAMPLES_ARRAY_SIZE));
                break;

            case 3:
                samplesBuffer = adc.getLastChannelSamples(3, false);
                for(uint8_t i = 0; i < SAMPLES_ARRAY_SIZE; i++)
                {
                    values[i] = adc.convertBitsToVoltageWithDCOffset(* samplesBuffer, VOLTAGE_DIVIDER_FACTOR, 3);
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
    }
    
    labResults["branch_A_cosphi"] = truncFloat3(cosphiMeassures[0]);
    labResults["branch_B_cosphi"] = truncFloat3(cosphiMeassures[1]);
    labResults["branch_C_cosphi"] = truncFloat3(cosphiMeassures[2]);

    return &labResults;
}

bool Lab::enableRelays()
{
    byteEncoder.setOutputByUInt(0, true); // to prevent relay triggering at power-on.
    byteEncoder.setOutputByUInt(0, true); // twice: one for shift and another for storage.
    byteEncoder.enableOutput(true); 
    return true;
}