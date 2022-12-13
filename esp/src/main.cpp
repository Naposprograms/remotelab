#include <Arduino.h>
#include <config.h>
#include <button.h>
#include <led.h>
#include <lab.h>
#include <WiFi.h>
#include <WebServer.h>


// Global objects
AliveLed ledAlive(LED_ALIVE_PIN, LED_ALIVE_TIME_ON, LED_ALIVE_TIME_OFF);
//SignalLed ledSignal(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
//Button sampleButton(PUSH_BUTTON_PIN, ACTIVE_LOW);
SignalLed ledWiFiConnected(LED_SIGNAL_PIN, LED_SIGNAL_TIME_ON, LED_SIGNAL_TIME_OFF);
Lab lab;
Timer calibrationTimer;
Timer labTimeout;


const char * ssid = PRIVATE_SSID;
const char * password = PRIVATE_PASSWORD;

bool adcOK = false;
bool relaysEnabled = false;
bool sampling = false;
bool calibrating = false;
bool correctLab;
bool firstLoop = true;
bool resultsReady = false;
uint32_t reconnectDelay = 1000;
bool wifiConnected = false;

DynamicJsonDocument * labJSON;

/**
 * BEGIN COMMUNICATION MODULE
*/

WebServer server(80);
void handlePost();
void sendMeasurement();

void setup_routing()
{
    server.on("/measurement", sendMeasurement);
    server.on("/options", HTTP_POST, handlePost);
    server.begin();    
}


// WiFi Connection Event
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("Connected to WiFi successfully!");
    wifiConnected = true;
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
    wifiConnected = false;
    Serial.print("No WiFi connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    Serial.println("Trying to connect");
    WiFi.begin(ssid, password);
    delay (reconnectDelay);
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
        #ifdef LAB_TYPE_P
            if(jsonDocument["lab"] == "parallel")
            {
                correctLab = true;
            }
        #endif
    #endif
    
    if(correctLab)
    {
        // if shortcircuit config sampling is true anyway
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
            labTimeout.set(LAB_TIMEOUT_MS);
        }
    }
    else
    {
        sampling = true;
        labTimeout.set(LAB_TIMEOUT_MS);
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


void sendMeasurement()
{
    if(sampling)
    {
        if(resultsReady)
        {
            String resultsJSON;
            if(!correctLab)
            {
                lab.writeErrorMsgToJSON("Wrong practice");
            }
            labJSON = lab.getLabResults(true, correctLab);
            serializeJsonPretty(* labJSON, resultsJSON);
            sampling = false;
            jsonDocument.clear();
            server.send(200, "application/json", resultsJSON);
        }
        else
        {
            server.send(200, "application/json", "busy");
        }
    }
    else
    {
        server.send(200, "application/json", "Lab config needed");
    }
}

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
    ledWiFiConnected.taskSignalLed();

    if(!wifiConnected)
    {
        ledWiFiConnected.blink(1);
    }
    
    if(sampling || calibrating)
    {
        if(lab.task()) // true when the lab is finished or the device is idle
        {
            if(sampling)
            {
                resultsReady = true;
                
                if(labTimeout.elapsed())
                {
                    lab.getLabResults(false, correctLab);
                    Serial.println("Lab results timed out");
                    sampling = false;
                }
                
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
                    reconnectDelay = RECONNECT_DELAY_MS;
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