#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "mqtt.h"

#include <CCTools.h>
extern CCTools CCTool;

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct MqttConfigStruct mqttCfg;

extern struct SysVarsStruct vars;
extern struct BrdConfigStruct hwConfig;

#include <AsyncMqttClient.h>
AsyncMqttClient mqttClient;

TimerHandle_t mqttReconnectTimer;
TimerHandle_t mqttPubStateTimer;

const char *homeAssistant = "homeassistant";
const char *haSensor = "sensor";
const char *haButton = "button";
const char *haBinarySensor = "binary_sensor";
const char *willMessage = "offline"; // to do

const char *availabilityTopic = "/avty";
const char *configTopic = "/config";
const char *stateTopic = "/state";

/* NO SSL SUPPORT in current SDK
#if ASYNC_TCP_SSL_ENABLED
#define MQTT_SECURE true
#define MQTT_SERVER_FINGERPRINT                                                                                                \
    {                                                                                                                          \
        0xAA, 0xD4, 0x06, 0x67, 0x05, 0xF2, 0xD3, 0x2E, 0xDD, 0x91, 0x76, 0x6F, 0xBE, 0xD5, 0xFB, 0xEC, 0x0A, 0x34, 0xC3, 0xBE \
    }
#endif
*/

String getUptime()
{
    String readableTime;
    getReadableTime(readableTime, 0);
    return readableTime;
}

String getTemperature()
{
    float CPUtemp = getCPUtemp();
    return String(CPUtemp);
}

String getWlanIp()
{
    return networkCfg.wifiDhcp ? WiFi.localIP().toString() : networkCfg.wifiIp.toString();
}

String getWlanSsid()
{
    return String(WiFi.SSID());
}

String getWlanRssi()
{
    return String(WiFi.RSSI());
}

String getLanIp()
{
    return networkCfg.ethDhcp ? ETH.localIP().toString() : networkCfg.ethIp.toString();
}

String getHostname()
{
    return systemCfg.hostname;
}

String getConnections()
{
    return String(vars.connectedClients);
}

String getZigbeeFWver()
{
    return String(CCTool.chip.fwRev);
}

String getZigbeeHWrev()
{
    return String(CCTool.chip.hwRev);
}

String getWorkMode()
{
    switch (systemCfg.workMode)
    {
    case WORK_MODE_USB:
        return "Zigbee-to-USB";
        break;
    case WORK_MODE_NETWORK:
        return "Zigbee-to-Network";
        break;
    }
    return "Error";
}

mqttTopicsConfig mqttTopicsConfigs[] = {
    {.name = "Restart ESP",
     .sensorType = haButton,
     .sensorId = "rst_esp",
     .stateTopic = "/io/rst_esp",
     .commandTopic = "/cmd",
     .icon = "mdi:restore-alert",
     .payloadPress = "{cmd:\"rst_esp\"}",
     .deviceClass = "restart"},
    {.name = "Restart Zigbee",
     .sensorType = haButton,
     .sensorId = "rst_zig",
     .stateTopic = "/io/rst_zig",
     .commandTopic = "/cmd",
     .icon = "mdi:restart",
     .payloadPress = "{cmd:\"rst_zig\"}",
     .deviceClass = "restart"},
    {.name = "Enable BSL",
     .sensorType = haButton,
     .sensorId = "enbl_bsl",
     .stateTopic = "/io/enbl_bsl",
     .commandTopic = "/cmd",
     .icon = "mdi:flash",
     .payloadPress = "{cmd:\"enbl_bsl\"}",
     .deviceClass = "identify"},
    {.name = "Socket",
     .sensorType = haBinarySensor,
     .sensorId = "socket",
     .stateTopic = "/io/socket",
     .deviceClass = "connectivity"},
    {.name = "Update ESP32",
     .sensorType = haBinarySensor,
     .sensorId = "update_esp",
     .stateTopic = "/io/update_esp",
     .deviceClass = "update"},
    {.name = "Uptime",
     .sensorType = haSensor,
     .sensorId = "uptime",
     .stateTopic = stateTopic,
     .icon = "mdi:clock",
     .valueTemplate = "{{ value_json.uptime }}",
     .getSensorValue = getUptime},
    {.name = "WLAN IP",
     .sensorType = haSensor,
     .sensorId = "wlanIp",
     .stateTopic = stateTopic,
     .icon = "mdi:wifi-check",
     .valueTemplate = "{{ value_json.wlanIp }}",
     .getSensorValue = getWlanIp},
    {.name = "WLAN SSID",
     .sensorType = haSensor,
     .sensorId = "wlanSsid",
     .stateTopic = stateTopic,
     .icon = "mdi:wifi-marker",
     .valueTemplate = "{{ value_json.wlanSsid }}",
     .getSensorValue = getWlanSsid},
    {.name = "WLAN RSSI",
     .sensorType = haSensor,
     .sensorId = "wlanRssi",
     .stateTopic = stateTopic,
     .valueTemplate = "{{ value_json.wlanRssi }}",
     .deviceClass = "signal_strength",
     .unitOfMeasurement = "dBm",
     .getSensorValue = getWlanRssi},
    {.name = "LAN IP",
     .sensorType = haSensor,
     .sensorId = "lanIp",
     .stateTopic = stateTopic,
     .icon = "mdi:check-network",
     .valueTemplate = "{{ value_json.lanIp }}",
     .getSensorValue = getLanIp},
    {.name = "ESP Temperature",
     .sensorType = haSensor,
     .sensorId = "temperature",
     .stateTopic = stateTopic,
     .icon = "mdi:coolant-temperature",
     .valueTemplate = "{{ value_json.temperature }}",
     .deviceClass = "temperature",
     .unitOfMeasurement = "°C",
     .getSensorValue = getTemperature},
    {.name = "Hostname",
     .sensorType = haSensor,
     .sensorId = "hostname",
     .stateTopic = stateTopic,
     .icon = "mdi:account-network",
     .valueTemplate = "{{ value_json.hostname }}",
     .getSensorValue = getHostname},
    {.name = "Socket Connections",
     .sensorType = haSensor,
     .sensorId = "connections",
     .stateTopic = stateTopic,
     .icon = "mdi:check-network-outline",
     .valueTemplate = "{{ value_json.connections }}",
     .getSensorValue = getConnections},
    {.name = "Mode",
     .sensorType = haSensor,
     .sensorId = "mode",
     .stateTopic = stateTopic,
     .icon = "mdi:access-point-network",
     .valueTemplate = "{{ value_json.mode }}",
     .deviceClass = "enum",
     .getSensorValue = getWorkMode},
    {.name = "Zigbee FW version",
     .sensorType = haSensor,
     .sensorId = "zbfw",
     .stateTopic = stateTopic,
     .icon = "mdi:access-point",
     .valueTemplate = "{{ value_json.zbfw }}",
     .getSensorValue = getZigbeeFWver},
    {.name = "Zigbee HW revision",
     .sensorType = haSensor,
     .sensorId = "zbhw",
     .stateTopic = stateTopic,
     .icon = "mdi:access-point",
     .valueTemplate = "{{ value_json.zbhw }}",
     .getSensorValue = getZigbeeHWrev}};

void mqttConnectSetup()
{
    LOGD("mqttConnectSetup");

    if (mqttCfg.reconnectInt == 0)
    {
        mqttCfg.reconnectInt = 30;
        LOGI("Reconnect Int didn't set. So use %d seconds", mqttCfg.reconnectInt);
    }
    if (mqttCfg.updateInt == 0)
    {
        mqttCfg.updateInt = 60;
        LOGI("Update Int didn't set. So use %d seconds", mqttCfg.updateInt);
    }
    mqttReconnectTimer = xTimerCreate("mqttReconnectTimer", pdMS_TO_TICKS(mqttCfg.reconnectInt * 1000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    mqttPubStateTimer = xTimerCreate("mqttPubStateTimer", pdMS_TO_TICKS(mqttCfg.updateInt * 1000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(mqttPublishState));

    if (mqttReconnectTimer == NULL)
    {
        LOGD("Failed to create mqttReconnectTimer");
    }

    if (mqttPubStateTimer == NULL)
    {
        LOGD("Failed to create mqttPubStateTimer");
    }

    uint16_t keepAlive = mqttCfg.updateInt + 10;
    mqttClient.setKeepAlive(keepAlive);

    const char* clientId = vars.deviceId;
    mqttClient.setClientId(clientId);

    mqttClient.setCredentials(mqttCfg.user, mqttCfg.pass);

    //String topic = String(mqttCfg.topic) + "/avty";
    //mqttClient.setWill(topic.c_str(), 1, true, "offline");

    mqttClient.setServer(mqttCfg.server, mqttCfg.port);

/* NO SSL SUPPORT in current SDK
#if ASYNC_TCP_SSL_ENABLED
    mqttClient.setSecure(MQTT_SECURE);
    if (MQTT_SECURE)
    {
        mqttClient.addServerFingerprint((const uint8_t[])MQTT_SERVER_FINGERPRINT);
    }
#endif
*/

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
    LOGD("Connecting to MQTT...");
    mqttClient.connect();
}

void onMqttConnect(bool sessionPresent)
{
    LOGD("Connected to MQTT. Session present: %s", String(sessionPresent).c_str());

    mqttClient.subscribe((String(mqttCfg.topic) + "/cmd").c_str(), 2);

    vars.mqttConn = true;

    mqttPublishDiscovery();

    mqttPublishIo("rst_esp", 0);
    mqttPublishIo("rst_zig", 0);
    mqttPublishIo("enbl_bsl", 0);
    bool socket_state = vars.connectedClients ? 1 : 0;
    mqttPublishIo("socket", socket_state);
    mqttPublishIo("update_esp", vars.updateEspAvail);
    mqttPublishState();

    mqttPublishAvail();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    LOGI("Disconnected from MQTT: %d", reason);

    vars.mqttConn = false;

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

void mqttPublishIo(const String &io, bool st)
{
    String state = st ? "ON" : "OFF";
    if (mqttClient.connected())
    {
        String topic = String(mqttCfg.topic) + "/io/" + io;
        LOGD("Pub Io %s at %s", state.c_str(), topic.c_str());
        mqttClient.publish(topic.c_str(), 0, true, state.c_str());
    }
}

void mqttPublishState()
{
    DynamicJsonDocument buffJson(512);
    for (const auto &item : mqttTopicsConfigs)
    {
        if (item.getSensorValue != nullptr)
        {
            String sensorValue = item.getSensorValue();
            if (sensorValue.length() > 0)
            {
                buffJson[item.sensorId] = sensorValue;
            }
        }
    }
    String topic = mqttCfg.topic + String(stateTopic);
    String mqttBuffer;
    serializeJson(buffJson, mqttBuffer);

    LOGD("%s", mqttBuffer.c_str());

    mqttClient.publish(topic.c_str(), 0, true, mqttBuffer.c_str());

    xTimerStart(mqttPubStateTimer, 0);
}

void mqttPublishDiscovery()
{
    DynamicJsonDocument devInfo(256);
    devInfo["ids"] = vars.deviceId;
    devInfo["name"] = systemCfg.hostname;
    devInfo["mf"] = "XZG";
    devInfo["mdl"] = hwConfig.board;
    char verArr[25];
    const char *env = STRINGIFY(BUILD_ENV_NAME);
    sprintf(verArr, "%s (%s)", VERSION, env);
    devInfo["sw"] = String(verArr);

    DynamicJsonDocument buffJson(2048);
    String mqttBuffer;

    for (const auto &item : mqttTopicsConfigs)
    {
        buffJson["dev"] = devInfo;
        buffJson["name"] = item.name;
        buffJson["uniq_id"] = String(mqttCfg.topic) + "/" + item.sensorId;
        buffJson["stat_t"] = mqttCfg.topic + item.stateTopic;
        buffJson["avty_t"] = mqttCfg.topic + String(availabilityTopic);
        if (!String(item.commandTopic).isEmpty())
        {
            buffJson["cmd_t"] = mqttCfg.topic + item.commandTopic;
        }

        if (!String(item.icon).isEmpty())
        {
            buffJson["icon"] = item.icon;
        }
        if (!String(item.payloadPress).isEmpty())
        {
            buffJson["payload_press"] = item.payloadPress;
        }
        if (!String(item.valueTemplate).isEmpty())
        {
            buffJson["val_tpl"] = item.valueTemplate;
        }
        if (!String(item.jsonAttributeTopic).isEmpty())
        {
            buffJson["json_attr_t"] = mqttCfg.topic + item.jsonAttributeTopic;
        }
        if (!String(item.deviceClass).isEmpty())
        {
            buffJson["dev_cla"] = item.deviceClass;
        }
        if (!String(item.unitOfMeasurement).isEmpty())
        {
            buffJson["unit_of_meas"] = item.unitOfMeasurement;
        }

        String topic = String(homeAssistant) + "/" + item.sensorType + "/" + mqttCfg.topic + "/" + item.sensorId + configTopic;
        serializeJson(buffJson, mqttBuffer);
        mqttClient.publish(topic.c_str(), 1, true, mqttBuffer.c_str());
        buffJson.clear();
        mqttBuffer.clear();
    }
}