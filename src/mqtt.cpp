#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <PubSubClient.h>
#include <esp_task_wdt.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "mqtt.h"

// extern struct ConfigSettingsStruct ConfigSettings;
extern struct zbVerStruct zbVer;
// extern struct MqttSettingsStruct MqttSettings;

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct MqttConfigStruct mqttCfg;

extern struct SysVarsStruct vars;

WiFiClient clientMqtt;

PubSubClient clientPubSub(clientMqtt);

String tagMQTT = "MQTT";

const char *homeAssistant = "homeassistant";
const char *haSensor = "sensor";
const char *haButton = "button";
const char *haBinarySensor = "binary_sensor";
const char *willMessage = "offline";

void mqttConnectSetup()
{
    LOGI(tagMQTT, "Try to connect %s:%d", mqttCfg.server, mqttCfg.port);
    if (strlen(mqttCfg.server) > 0 && mqttCfg.port > 0)
    {
        clientPubSub.setServer(mqttCfg.server, mqttCfg.port); //
        LOGI(tagMQTT, "Try to setCallback");
        clientPubSub.setCallback(mqttCallback);
    }
    else
    {
        LOGE(tagMQTT, "Error in config");
    }
}

bool mqttReconnect()
{
    LOGI(tagMQTT, "Attempting MQTT connection...");

    byte willQoS = 0;
    String willTopic = String(mqttCfg.topic) + "/avty";

    boolean willRetain = false;
    //char deviceIdArr[MAX_DEV_ID_LONG];
    //getDeviceID(deviceIdArr);

    if (clientPubSub.connect(vars.deviceId, mqttCfg.user, mqttCfg.pass, willTopic.c_str(), willQoS, willRetain, willMessage))
    {
        vars.mqttReconnectTime = 0;
        mqttOnConnect();
        LOGI(tagMQTT, "OK, %d", clientPubSub.state());
        return true;
    }
    else
    {
        LOGI(tagMQTT, "failed, rc= %d  try again in %d seconds", clientPubSub.state(), mqttCfg.reconnectInt);

        return false;
    }
}

void mqttOnConnect()
{
    //LOGI(tagMQTT, "connected");
    vars.mqttConn = true;
    mqttSubscribe("cmd");
    LOGI(tagMQTT, "mqtt Subscribed");
    if (mqttCfg.discovery)
    {
        mqttPublishDiscovery();
        LOGI(tagMQTT, "mqtt Published Discovery");
    }

    mqttPublishIo("rst_esp", "OFF");
    mqttPublishIo("rst_zig", "OFF");
    mqttPublishIo("enbl_bsl", "OFF");
    mqttPublishIo("socket", "OFF");
    LOGI(tagMQTT, "mqtt Published IOs");
    mqttPublishAvail();
    LOGI(tagMQTT, "mqtt Published Avty");
    if (mqttCfg.updateInt > 0)
    {
        mqttPublishState();
        LOGI(tagMQTT, "mqtt Published State");
    }
}

void mqttPublishMsg(String topic, String msg, bool retain)
{
    clientPubSub.beginPublish(topic.c_str(), msg.length(), retain);
    clientPubSub.print(msg.c_str());
    clientPubSub.endPublish();
}

void mqttPublishAvail()
{
    String topic(mqttCfg.topic);
    topic = topic + "/avty";
    String mqttBuffer = "online";
    clientPubSub.publish(topic.c_str(), mqttBuffer.c_str(), true);
}

void mqttPublishState()
{
    String topic(mqttCfg.topic);
    topic = topic + "/state";
    DynamicJsonDocument root(1024);
    String readableTime;
    getReadableTime(readableTime, 0);
    root["uptime"] = readableTime;

    float CPUtemp = getCPUtemp();
    root["temperature"] = String(CPUtemp);

    root["connections"] = vars.connectedClients;

    if (vars.connectedEther)
    {
        if (networkCfg.ethDhcp)
        {
            root["ip"] = ETH.localIP().toString();
        }
        else
        {
            root["ip"] = networkCfg.ethIp;
        }
    }
    else
    {
        if (networkCfg.wifiDhcp)
        {
            root["ip"] = WiFi.localIP().toString();
        }
        else
        {
            root["ip"] = networkCfg.wifiIp;
        }
    }
    // if (ConfigSettings.emergencyWifi)
    //{
    //     root["emergencyMode"] = "ON";
    // }
    // else
    //{
    //     root["emergencyMode"] = "OFF";
    // }

    switch (systemCfg.workMode)
    {
    case WORK_MODE_USB:
        root["mode"] = "Zigbee-to-USB";
        break;
    /*case COORDINATOR_MODE_WIFI:
        root["mode"] = "Zigbee-to-WiFi";
        break;*/
    case WORK_MODE_NETWORK:
        root["mode"] = "Zigbee-to-Network";
        break;
    default:
        break;
    }
    root["zbfw"] = String(zbVer.zbRev);
    root["hostname"] = systemCfg.hostname;
    String mqttBuffer;
    serializeJson(root, mqttBuffer);
    // DEBUG_PRINTLN(mqttBuffer);
    clientPubSub.publish(topic.c_str(), mqttBuffer.c_str(), true);
    vars.mqttHeartbeatTime = millis() + (mqttCfg.updateInt * 1000);
}

void mqttPublishIo(String const &io, String const &state)
{
    if (clientPubSub.connected())
    {
        String topic(mqttCfg.topic);
        topic = topic + "/io/" + io;
        clientPubSub.publish(topic.c_str(), state.c_str(), true);
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    char jjson[length + 1];
    memcpy(jjson, payload, length);
    //jjson[length + 1] = '\0';
    jjson[length] = '\0';

    DynamicJsonDocument jsonBuffer(512);

    deserializeJson(jsonBuffer, jjson);

    const char *command = jsonBuffer["cmd"];

    LOGI(tagMQTT, "mqtt Callback - %s", String(jjson));

    if (command)
    {
        LOGI(tagMQTT, "mqtt cmd - %s", String(command));
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
    String mtopic(mqttCfg.topic);
    mtopic = mtopic + "/" + topic;
    clientPubSub.subscribe(mtopic.c_str());
}

void mqttLoop()
{
    if (!clientPubSub.connected())
    {
        vars.mqttConn = false;
        if (vars.mqttReconnectTime == 0)
        {
            // mqttReconnect();
            LOGI(tagMQTT, "Connect in %d seconds", mqttCfg.reconnectInt);
            vars.mqttReconnectTime = millis() + mqttCfg.reconnectInt*1000;
        }
        else
        {
            if (vars.mqttReconnectTime <= millis())
            {
                bool conn = mqttReconnect();
                //LOGI(tagMQTT, "mqttReconnect exit with %s", conn ? "true" : "false");
                vars.mqttReconnectTime = millis() + mqttCfg.reconnectInt*1000;
            }
        }
    }
    else
    {
        clientPubSub.loop();
        if (mqttCfg.updateInt > 0)
        {
            if (vars.mqttHeartbeatTime <= millis())
            {
                mqttPublishState();
            }
        }
    }
    // LOGI(tagMQTT, "loop end");
}

void mqttPublishDiscovery()
{
    String mtopic(mqttCfg.topic);
    String deviceName = mtopic; // systemCfg.hostname;

    String topic;
    DynamicJsonDocument buffJson(2048);
    String mqttBuffer;

    DynamicJsonDocument via(150);
    //char deviceIdArr[MAX_DEV_ID_LONG];
    //getDeviceID(deviceIdArr);
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
            //char deviceIdArr[MAX_DEV_ID_LONG];
            //getDeviceID(deviceIdArr);

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
            // DEBUG_PRINTLN(mqttBuffer);
            mqttPublishMsg(topic, mqttBuffer, true);
            buffJson.clear();
            mqttBuffer = "";
        }
        else
        {
            i = 100;
        }
    }
}