#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include "WiFi.h"
#include <WebServer.h>
#include "FS.h"
#include "LITTLEFS.h"
#include "web.h"
#include "config.h"
#include "log.h"
#include "etc.h"
#include <PubSubClient.h>

extern struct ConfigSettingsStruct ConfigSettings;

WiFiClient clientMqtt;

PubSubClient clientPubSub(clientMqtt);

//IPAddress serverMqtt (10,0,10,111);

byte willQoS = 0;
const char* willTopic = "willTopic";
const char* willMessage = "My Will Message";
boolean willRetain = false;

void reconnectMqtt()
{

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (clientPubSub.connect("arduinoClient", ConfigSettings.mqttUser, ConfigSettings.mqttPass, willTopic, willQoS, willRetain, willMessage))
    //if (clientPubSub.connect("arduinoClient", "", ""))
    {
        ConfigSettings.mqttReconnectTime = 0;
        Serial.println("connected");
        // Once connected, publish an announcement...
        clientPubSub.publish("outTopic", "hello world");
        // ... and resubscribe
        clientPubSub.subscribe("inTopic");
    }
    else
    {
        Serial.print("failed, rc=");
        Serial.print(clientPubSub.state());
        Serial.println(" try again in 5 seconds");
        ConfigSettings.mqttReconnectTime = millis() + 5000;
    }
}

void callbackMqtt(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
}

void mqttConnectSetup()
{
    clientPubSub.setServer(ConfigSettings.mqttServerIP, ConfigSettings.mqttPort);
    clientPubSub.setCallback(callbackMqtt);
}

void mqttLoop()
{
    if (!clientPubSub.connected())
    {
        if (ConfigSettings.mqttReconnectTime == 0)
        {
            reconnectMqtt();
        }
        else
        {
            if (ConfigSettings.mqttReconnectTime <= millis())
            {   
                reconnectMqtt();
            }
        }
    }
    else
    {
        clientPubSub.loop();
    }
}