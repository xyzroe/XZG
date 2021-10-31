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
#include "mqtt.h"

extern struct ConfigSettingsStruct ConfigSettings;

WiFiClient clientMqtt;

PubSubClient clientPubSub(clientMqtt);

void mqttConnectSetup()
{
    clientPubSub.setServer(ConfigSettings.mqttServerIP, ConfigSettings.mqttPort);
    clientPubSub.setCallback(mqttCallback);
}

void mqttReconnect()
{
    DEBUG_PRINT(F("Attempting MQTT connection..."));

    byte willQoS = 0;
    String willTopic = String(ConfigSettings.mqttTopic) + "/avty";
    const char *willMessage = "offline";
    boolean willRetain = false;
    String clientID;
    getDeviceID(clientID);

    if (clientPubSub.connect(clientID.c_str(), ConfigSettings.mqttUser, ConfigSettings.mqttPass, willTopic.c_str(), willQoS, willRetain, willMessage))
    {
        ConfigSettings.mqttReconnectTime = 0;
        mqttOnConnect();
    }
    else
    {
        DEBUG_PRINT(F("failed, rc="));
        DEBUG_PRINT(clientPubSub.state());
        DEBUG_PRINT(F(" try again in "));
        DEBUG_PRINT(ConfigSettings.mqttInterval);
        DEBUG_PRINTLN(F(" seconds"));

        ConfigSettings.mqttReconnectTime = millis() + ConfigSettings.mqttInterval * 1000;
    }
}

void mqttOnConnect()
{
    DEBUG_PRINTLN(F("connected"));
    mqttSubscribe("cmd");
    DEBUG_PRINTLN(F("mqtt Subscribed"));
    if (ConfigSettings.mqttInterval > 0)
    {
        mqttPublishState();
        DEBUG_PRINTLN(F("mqtt Published State"));
    }
    if (ConfigSettings.mqttDiscovery)
    {
        mqttPublishDiscovery();
        DEBUG_PRINTLN(F("mqtt Published Discovery"));
    }
    mqttPublishIo("rst_esp", "OFF");
    mqttPublishIo("rst_zig", "OFF");
    mqttPublishIo("enbl_bsl", "OFF");
    mqttPublishIo("socket", "OFF");
    DEBUG_PRINTLN(F("mqtt Published State"));
    mqttPublishAvty();
    DEBUG_PRINTLN(F("mqtt Published Avty"));
}

void mqttPublishMsg(String topic, String msg, bool retain)
{
    clientPubSub.beginPublish(topic.c_str(), msg.length(), retain);
    clientPubSub.print(msg.c_str());
    clientPubSub.endPublish();
}

void mqttPublishAvty()
{
    String topic(ConfigSettings.mqttTopic);
    topic = topic + "/avty";
    String mqttBuffer = "online";
    clientPubSub.publish(topic.c_str(), mqttBuffer.c_str(), true);
}

void mqttPublishState()
{
    String topic(ConfigSettings.mqttTopic);
    topic = topic + "/state";
    DynamicJsonDocument root(1024);
    String readableTime;
    getReadableTime(readableTime, 0);
    root["uptime"] = readableTime;
    String CPUtemp;
    getCPUtemp(CPUtemp);
    root["temperature"] = CPUtemp;
    /*
    if (ConfigSettings.connectedSocket)
    {
        root["socket"] = "ON";
    }
    else
    {
        root["socket"] = "OFF";
    }
    */
    if (ConfigSettings.connectedEther)
    {
        if (ConfigSettings.dhcp)
        {
            root["ip"] = ETH.localIP().toString();
        }
        else
        {
            root["ip"] = ConfigSettings.ipAddress;
        }
    }
    else
    {
        if (ConfigSettings.dhcpWiFi)
        {
            root["ip"] = WiFi.localIP().toString();
        }
        else
        {
            root["ip"] = ConfigSettings.ipAddressWiFi;
        }
    }
    if (ConfigSettings.emergencyWifi)
    {
        root["emergencyMode"] = "ON";
    }
    else
    {
        root["emergencyMode"] = "OFF";
    }
    root["hostname"] = ConfigSettings.hostname;
    String mqttBuffer;
    serializeJson(root, mqttBuffer);
    DEBUG_PRINTLN(mqttBuffer);
    clientPubSub.publish(topic.c_str(), mqttBuffer.c_str());
    ConfigSettings.mqttHeartbeatTime = millis() + (ConfigSettings.mqttInterval * 1000);
}

void mqttPublishIo(String const &io, String const &state)
{
    if (clientPubSub.connected())
    {
        String topic(ConfigSettings.mqttTopic);
        topic = topic + "/io/" + io;
        clientPubSub.publish(topic.c_str(), state.c_str());
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char jjson[length + 1];
    memcpy(jjson, payload, length);
    jjson[length + 1] = '\0';

    DynamicJsonDocument jsonBuffer(1024);

    deserializeJson(jsonBuffer, jjson);

    const char *command = jsonBuffer["cmd"];

    DEBUG_PRINT(F("mqtt Callback - "));
    DEBUG_PRINTLN(jjson);

    if (command) {
        DEBUG_PRINT(F("mqtt cmd - "));
        DEBUG_PRINTLN(command);
        if (strcmp(command, "rst_esp") == 0)
        {
            printLogMsg("ESP restart MQTT");
            ESP.restart();
        }

        if (strcmp(command, "rst_zig") == 0)
        {
            printLogMsg("Zigbee restart MQTT");
            zigbeeRestart();
        }

        if (strcmp(command, "enbl_bsl") == 0)
        {
            printLogMsg("Zigbee BSL enable MQTT");
            zigbeeEnableBSL();
        }
    }
    return;
}

void mqttSubscribe(String topic)
{
    String mtopic(ConfigSettings.mqttTopic);
    mtopic = mtopic + "/" + topic;
    clientPubSub.subscribe(mtopic.c_str());
}

void mqttLoop()
{
    if (!clientPubSub.connected())
    {
        if (ConfigSettings.mqttReconnectTime == 0)
        {
            //mqttReconnect();
            DEBUG_PRINTLN(F("mqttReconnect in 5 seconds"));
            ConfigSettings.mqttReconnectTime = millis() + 5000;
        }
        else
        {
            if (ConfigSettings.mqttReconnectTime <= millis())
            {
                mqttReconnect();
            }
        }
    }
    else
    {
        clientPubSub.loop();
        if (ConfigSettings.mqttInterval > 0)
        {
            if (ConfigSettings.mqttHeartbeatTime <= millis())
            {
                mqttPublishState();
            }
        }
    }
}

void mqttPublishDiscovery()
{
    String mtopic(ConfigSettings.mqttTopic);
    String deviceName = mtopic; //ConfigSettings.hostname;

    String topic;
    DynamicJsonDocument buffJson(2048);
    String mqttBuffer;

    DynamicJsonDocument via(1024);
    via["ids"] = ETH.macAddress();

    for (int i = 0; i < 9; i++)
    {
        switch (i)
        {
        case 0:
        {
            DynamicJsonDocument dev(1024);
            dev["ids"] = ETH.macAddress();
            dev["name"] = ConfigSettings.hostname;
            dev["mf"] = "Zig Star";
            dev["mdl"] = ConfigSettings.boardName;
            dev["sw"] = VERSION;

            topic = "homeassistant/switch/" + deviceName + "/rst_esp/config";
            buffJson["name"] = "Restart ESP";
            buffJson["uniq_id"] = deviceName + "/rst_esp";
            buffJson["stat_t"] = mtopic + "/io/rst_esp";
            buffJson["cmd_t"] = mtopic + "/cmd";
            buffJson["icon"] = "mdi:restore-alert";
            buffJson["pl_on"] = "{cmd:\"rst_esp\"}";
            buffJson["pl_off"] = "{cmd:\"rst_esp\"}";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev"] = dev;
            break;
        }
        case 1:
        {
            topic = "homeassistant/switch/" + deviceName + "/rst_zig/config";
            buffJson["name"] = "Restart Zigbee";
            buffJson["uniq_id"] = deviceName + "/rst_zig";
            buffJson["stat_t"] = mtopic + "/io/rst_zig";
            buffJson["cmd_t"] = mtopic + "/cmd";
            buffJson["icon"] = "mdi:restart";
            buffJson["pl_on"] = "{cmd:\"rst_zig\"}";
            buffJson["pl_off"] = "{cmd:\"rst_zig\"}";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev"] = via;
            break;
        }
        case 2:
        {
            topic = "homeassistant/switch/" + deviceName + "/enbl_bsl/config";
            buffJson["name"] = "Enable BSL";
            buffJson["uniq_id"] = deviceName + "/enbl_bsl";
            buffJson["stat_t"] = mtopic + "/io/enbl_bsl";
            buffJson["cmd_t"] = mtopic + "/cmd";
            buffJson["icon"] = "mdi:flash";
            buffJson["pl_on"] = "{cmd:\"enbl_bsl\"}";
            buffJson["pl_off"] = "{cmd:\"enbl_bsl\"}";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev"] = via;
            break;
        }
        case 3:
        {
            topic = "homeassistant/binary_sensor/" + deviceName + "/socket/config";
            buffJson["name"] = "Socket";
            buffJson["uniq_id"] = deviceName + "/socket";
            buffJson["stat_t"] = mtopic + "/io/socket";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev_cla"] = "connectivity";
            buffJson["dev"] = via;
            break;
        }
        case 4:
        {
            topic = "homeassistant/binary_sensor/" + deviceName + "/emrgncMd/config";
            buffJson["name"] = "Emergency mode";
            buffJson["uniq_id"] = deviceName + "/emrgncMd";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.emergencyMode }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["dev_cla"] = "power";
            buffJson["icon"] = "mdi:access-point-network";
            buffJson["dev"] = via;
            break;
        }
        case 5:
        {
            topic = "homeassistant/sensor/" + deviceName + "/uptime/config";
            buffJson["name"] = "Uptime";
            buffJson["uniq_id"] = deviceName + "/uptime";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.uptime }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:clock";
            buffJson["dev"] = via;
            break;
        }
        case 6:
        {
            topic = "homeassistant/sensor/" + deviceName + "/ip/config";
            buffJson["name"] = "IP";
            buffJson["uniq_id"] = deviceName + "/ip";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.ip }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:check-network";
            buffJson["dev"] = via;
            break;
        }
        case 7:
        {
            topic = "homeassistant/sensor/" + deviceName + "/temperature/config";
            buffJson["name"] = "CPU temperature";
            buffJson["uniq_id"] = deviceName + "/temperature";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.temperature }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:coolant-temperature";
            buffJson["dev"] = via;
            break;
        }
        case 8:
        {
            topic = "homeassistant/sensor/" + deviceName + "/hostname/config";
            buffJson["name"] = "Hostname";
            buffJson["uniq_id"] = deviceName + "/hostname";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.hostname }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:account-network";
            buffJson["dev"] = via;
            break;
        }
        }
        serializeJson(buffJson, mqttBuffer);
        DEBUG_PRINTLN(mqttBuffer);
        mqttPublishMsg(topic, mqttBuffer, true);
        buffJson.clear();
        mqttBuffer = "";
    }
}