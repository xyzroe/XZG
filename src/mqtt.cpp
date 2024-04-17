#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <AsyncMqttClient.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "mqtt.h"

extern struct zbVerStruct zbVer;

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct MqttConfigStruct mqttCfg;

extern struct SysVarsStruct vars;

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t mqttPubStateTimer;

String tagMQTT = "MQTT";

const char *homeAssistant = "homeassistant";
const char *haSensor = "sensor";
const char *haButton = "button";
const char *haBinarySensor = "binary_sensor";
const char *willMessage = "offline";

void mqttConnectSetup()
{
    LOGD("mqttConnectSetup");

    mqttReconnectTimer = xTimerCreate("mqttReconnectTimer", pdMS_TO_TICKS(mqttCfg.reconnectInt * 1000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    mqttPubStateTimer = xTimerCreate("mqttPubStateTimer", pdMS_TO_TICKS(mqttCfg.updateInt * 1000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(mqttPublishState));
    
    if (mqttReconnectTimer == NULL)
    {
        LOGD("Failed to create timer");
    }
    else
    {
        LOGD("Timer created successfully");
    }
    mqttClient.setServer(mqttCfg.server, mqttCfg.port);
    mqttClient.setCredentials(mqttCfg.user, mqttCfg.pass);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);

    mqttClient.onMessage(onMqttMessage);

    if (xTimerStart(mqttReconnectTimer, 0) != pdPASS)
    {
        LOGD("Failed to start timer");
    }
}

void connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void onMqttConnect(bool sessionPresent)
{
    LOGD("Connected to MQTT. Session present: %s", String(sessionPresent).c_str());

    mqttClient.subscribe((String(mqttCfg.topic) + "/cmd").c_str(), 2);

    mqttPublishDiscovery();
    
    mqttPublishIo("rst_esp", "OFF");
    mqttPublishIo("rst_zig", "OFF");
    mqttPublishIo("enbl_bsl", "OFF");
    mqttPublishIo("socket", "OFF");
    mqttPublishState();

    mqttPublishAvail();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    LOGP("Disconnected from MQTT: %d", reason);
    xTimerStart(mqttReconnectTimer, 0);
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    String topicStr(topic); 
    if (topicStr.endsWith("/cmd"))
    {
        char json[len + 1];
        memcpy(json, payload, len);
        json[len] = '\0';

        DynamicJsonDocument doc(512);
        deserializeJson(doc, json);
        const char *command = doc["cmd"];

        LOGD("cmd - %s", String(json));

        if (command)
        {
            executeCommand(command);
        }
    }
}

void executeCommand(const char *command)
{
    LOGD("mqtt cmd - %s", String(command));
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

void mqttPublishAvail()
{
    String topic = String(mqttCfg.topic) + "/avty";
    mqttClient.publish(topic.c_str(), 1, true, "online");
}

void mqttPublishIo(const String &io, const String &state)
{
    if (mqttClient.connected())
    {
        String topic = String(mqttCfg.topic) + "/io/" + io;
        LOGD("Pub Io %s at %s", state.c_str(), topic.c_str());
        mqttClient.publish(topic.c_str(), 0, true, state.c_str());
    }
}

void mqttPublishState()
{
    String topic = String(mqttCfg.topic) + "/state";
    DynamicJsonDocument root(1024);

    String readableTime;
    getReadableTime(readableTime, 0);
    root["uptime"] = readableTime;

    float CPUtemp = getCPUtemp();
    root["temperature"] = String(CPUtemp);
    root["connections"] = vars.connectedClients;

    String ip = networkCfg.wifiDhcp ? WiFi.localIP().toString() : networkCfg.wifiIp.toString();
    root["ip"] = ip;

    switch (systemCfg.workMode)
    {
    case WORK_MODE_USB:
        root["mode"] = "Zigbee-to-USB";
        break;
    case WORK_MODE_NETWORK:
        root["mode"] = "Zigbee-to-Network";
        break;
    }

    root["zbfw"] = String(zbVer.zbRev);
    root["hostname"] = systemCfg.hostname;

    String mqttBuffer;
    serializeJson(root, mqttBuffer);

    mqttClient.publish(topic.c_str(), 0, true, mqttBuffer.c_str());


    xTimerStart(mqttPubStateTimer, 0);
}

void mqttPublishDiscovery()
{
    String mtopic(mqttCfg.topic);
    String deviceName = mtopic; // systemCfg.hostname;

    String topic;
    DynamicJsonDocument buffJson(2048);
    String mqttBuffer;

    DynamicJsonDocument via(150);
    // char deviceIdArr[MAX_DEV_ID_LONG];
    // getDeviceID(deviceIdArr);
    via["ids"] = String(vars.deviceId);

    String sensor_topic = String(homeAssistant) + "/" + String(haSensor) + "/";
    String button_topic = String(homeAssistant) + "/" + String(haButton) + "/";
    String binary_sensor_topic = String(homeAssistant) + "/" + String(haBinarySensor) + "/";
    // int lastAutoMsg = 9;
    // if (ConfigSettings.board == 2) lastAutoMsg--;

    for (int i = 0; i <= 99; i++)
    {
        switch (i)
        {
        case 0:
        {
            DynamicJsonDocument dev(256);
            // char deviceIdArr[MAX_DEV_ID_LONG];
            // getDeviceID(deviceIdArr);

            dev["ids"] = String(vars.deviceId);
            dev["name"] = systemCfg.hostname;
            dev["mf"] = "Zig Star";
            // dev["mdl"] = ConfigSettings.boardName;

            char verArr[25];
            const char *env = STRINGIFY(BUILD_ENV_NAME);
            sprintf(verArr, "%s (%s)", VERSION, env);
            dev["sw"] = String(verArr);

            topic = button_topic + deviceName + "/rst_esp/config";
            buffJson["name"] = "Restart ESP";
            buffJson["uniq_id"] = deviceName + "/rst_esp";
            buffJson["stat_t"] = mtopic + "/io/rst_esp";
            buffJson["cmd_t"] = mtopic + "/cmd";
            buffJson["icon"] = "mdi:restore-alert";
            buffJson["payload_press"] = "{cmd:\"rst_esp\"}";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev"] = dev;
            break;
        }
        case 1:
        {
            topic = button_topic + deviceName + "/rst_zig/config";
            buffJson["name"] = "Restart Zigbee";
            buffJson["uniq_id"] = deviceName + "/rst_zig";
            buffJson["stat_t"] = mtopic + "/io/rst_zig";
            buffJson["cmd_t"] = mtopic + "/cmd";
            buffJson["icon"] = "mdi:restart";
            buffJson["payload_press"] = "{cmd:\"rst_zig\"}";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev"] = via;
            break;
        }
        case 2:
        {
            topic = button_topic + deviceName + "/enbl_bsl/config";
            buffJson["name"] = "Enable BSL";
            buffJson["uniq_id"] = deviceName + "/enbl_bsl";
            buffJson["stat_t"] = mtopic + "/io/enbl_bsl";
            buffJson["cmd_t"] = mtopic + "/cmd";
            buffJson["icon"] = "mdi:flash";
            buffJson["payload_press"] = "{cmd:\"enbl_bsl\"}";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev"] = via;
            break;
        }
        case 3:
        {
            topic = binary_sensor_topic + deviceName + "/socket/config";
            buffJson["name"] = "Socket";
            buffJson["uniq_id"] = deviceName + "/socket";
            buffJson["stat_t"] = mtopic + "/io/socket";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["dev_cla"] = "connectivity";
            buffJson["dev"] = via;
            break;
        }
        // case 4:
        //{
        //     topic = binary_sensor_topic + deviceName + "/emrgncMd/config";
        //     buffJson["name"] = "Emergency mode";
        //     buffJson["uniq_id"] = deviceName + "/emrgncMd";
        //     buffJson["stat_t"] = mtopic + "/state";
        //     buffJson["avty_t"] = mtopic + "/avty";
        //     buffJson["val_tpl"] = "{{ value_json.emergencyMode }}";
        //     buffJson["json_attr_t"] = mtopic + "/state";
        //     buffJson["dev_cla"] = "power";
        //     buffJson["icon"] = "mdi:access-point-network";
        //     buffJson["dev"] = via;
        //     break;
        // }
        case 4:
        {
            topic = sensor_topic + deviceName + "/uptime/config";
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
        case 5:
        {
            topic = sensor_topic + deviceName + "/ip/config";
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
        case 6:
        {
            topic = sensor_topic + deviceName + "/temperature/config";
            buffJson["name"] = "ESP temperature";
            buffJson["uniq_id"] = deviceName + "/temperature";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.temperature }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:coolant-temperature";
            buffJson["dev"] = via;
            buffJson["dev_cla"] = "temperature";
            buffJson["stat_cla"] = "measurement";
            buffJson["unit_of_meas"] = "°C";
            break;
        }
        case 7:
        {
            topic = sensor_topic + deviceName + "/hostname/config";
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
        case 8:
        {
            topic = sensor_topic + deviceName + "/connections/config";
            buffJson["name"] = "Socket connections";
            buffJson["uniq_id"] = deviceName + "/connections";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.connections }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:check-network-outline";
            buffJson["dev"] = via;
            break;
        }
        case 9:
        {
            topic = sensor_topic + deviceName + "/mode/config";
            buffJson["name"] = "Mode";
            buffJson["uniq_id"] = deviceName + "/mode";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.mode }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:access-point-network";
            buffJson["dev"] = via;
            break;
        }
        case 10:
        {
            topic = sensor_topic + deviceName + "/zbfw/config";
            buffJson["name"] = "Zigbee fw rev";
            buffJson["uniq_id"] = deviceName + "/zbfw";
            buffJson["stat_t"] = mtopic + "/state";
            buffJson["avty_t"] = mtopic + "/avty";
            buffJson["val_tpl"] = "{{ value_json.zbfw }}";
            buffJson["json_attr_t"] = mtopic + "/state";
            buffJson["icon"] = "mdi:access-point-network";
            buffJson["dev"] = via;
            break;
        }
        default:
            topic = "error";
            break;
        }
        if (topic != "error")
        {
            serializeJson(buffJson, mqttBuffer);
            mqttClient.publish(topic.c_str(), 1, true, mqttBuffer.c_str());

            buffJson.clear();
            mqttBuffer = "";
        }
        else
        {
            i = 100;
        }
    }
}