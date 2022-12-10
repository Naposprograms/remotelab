#include <Arduino.h>
#include <config.h>
#include <button.h>
#include <led.h>
#include <lab.h>
#include <WiFi.h>
#include <WebServer.h>


// Global objects
AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Button sampleButton(PUSH_BUTTON_PIN, ACTIVE_LOW);
Lab lab;
Timer calibrationTimer;

const char * ssid = PRIVATE_SSID;
const char * password = PRIVATE_PASSWORD;

bool adcOK = false;
bool relaysEnabled = false;
bool sampling = false;
bool calibrating = false;
bool correctLab;
bool firstLoop = true;

DynamicJsonDocument * labJSON;

/**
 * BEGIN COMMUNICATION MODULE
*/
WebServer server(80);
void handlePost();

void setup_routing()
{
    //server.on("/measurement", sendMeasurement);
    server.on("/options", HTTP_POST, handlePost);
    server.begin();    
}


// WiFi Connection Event
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("Connected to WiFi successfully!");
}

// Got IP Event
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    setup_routing();
}

// WiFI Disconnection Event
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.print("No WiFi connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println("Trying to connect");
    WiFi.begin(ssid, password);
    //delay (1000);
}



StaticJsonDocument <512> jsonDocument;
void handlePost()
{
    if (server.hasArg("plain") == false) {
        server.send(200, "text/plain", "Body not received");
        return;
    }

    deserializeJson(jsonDocument, server.arg("plain"));

    
    correctLab = false;
    #ifdef LAB_TYPE_S
        if(jsonDocument["lab"] == "series")
        {
            correctLab = true;
        }
    #else
        if(jsonDocument["lab"] == "parallel")
        {
            correctLab = true;
        }
    #endif    


    String config = jsonDocument["circuitconfig"];
    char labConfig[9];
    config.toCharArray(&labConfig[0], 9, 0); 
    Serial.printf("Lab config: %s\n", labConfig);
    if(lab.checkBusy())
    {
        Serial.println("Lab Busy");
    }
    else
    {
        sampling = lab.doLab(&labConfig[0]);
    }
    Serial.printf("Sampling: %d\n", sampling);

    server.send(200, "application/json", "{}");
}

void initWifi()
{
    WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.println("Wait for WiFi... ");
    WiFi.begin(ssid, password);
}


/*

void sendMeasurement()
{
    jsonDocument.clear();
    add_json_object(lab, circuitconfig, node_A_RMS, node_A_values[0], node_B_RMS, node_B_values[0], node_C_RMS, node_C_values[0], current_RMS, current_values[0], node_A_cosphi, node_B_cosphi, node_C_cosphi);
    serializeJson(jsonDocument, buffer);
    //server.send(200, "application/json", buffer);
    //reemplzar buffer por serializeJSON
    ;
}

*/

/**
 * END COMMUNICATION MODULE
*/

void setup()
{
    WiFi.disconnect(true);
    delay(500); // delays on power ON to allow caps to charge and voltage levels to stabilize.
    Serial.begin(BAUD_RATE);
    relaysEnabled = lab.enableRelays();
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
            Serial.println("Lab task true");
            if(sampling)
            {
                labJSON = lab.getLabResults(false, correctLab);
                serializeJsonPretty(* labJSON, Serial);
                sampling = false;
            }
            if(calibrating)
            {
                float * ptr = lab.getADCDCOffsetVolts();
                for(uint8_t chNumber = 0; chNumber < 4; chNumber++)
                {
                    Serial.printf("CH%d DC offset: %f\n", chNumber, * (ptr + chNumber));
                }
                calibrating = false;
                calibrationTimer.set(TIME_BETWEEN_CALIBRATIONS_MS);
                if(firstLoop)
                {
                    initWifi();
                    firstLoop = false;
                }
            }
        }
    }
    else
    {
        if(calibrationTimer.elapsed() && !calibrating)
        {
            lab.calibrateADC();
            calibrating = true;
        }
    }

    /*

    if(sampleButton.pressed())
    {
        ledSignal.blink(3);
        Serial.print("Enter circuit config > ");
        enteredConfig = false;
    }
    */

    /*
    if(!enteredConfig)
    {
        if(Serial.available() > 8) // expected config has 8 chars
        {
            String config = "";
            config = Serial.readStringUntil('\n');
            config.replace('\r', '\0');
            config = config.substring(0, 8);
            enteredConfig = true;
            //Serial.printf("Config read: %s\n", config);
            char labConfig[9];
            config.toCharArray(&labConfig[0], 9, 0); 
            Serial.printf("Lab config: %s\n", labConfig);
            sampling = lab.doLab(&labConfig[0]);
        }
    }
    */

    server.handleClient();
}