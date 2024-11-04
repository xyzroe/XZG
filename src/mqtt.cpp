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
extern struct ThisConfigStruct hwConfig;

#include <AsyncMqttClient.h>
AsyncMqttClient mqttClient;

TimerHandle_t mqttReconnectTimer;
TimerHandle_t mqttPubStateTimer;

const char *homeAssistant = "homeassistant";
const char *haSensor = "sensor";
const char *haUpdate = "update";
const char *haButton = "button";
const char *haBinarySensor = "binary_sensor";
const char *willMessage = "offline"; // to do
const char *haUpdateLogoUrl = "https://xzg.xyzroe.cc/assets/images/logo.svg";
const char *haUpdateAttrTemplate = "{\"in_progress\": {{ value_json['state'] }} }";

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

String getStrCpuTemp()
{
    float CPUtemp = getCPUtemp();
    return String(CPUtemp);
}

String getStr1wTemp()
{
    float temp = get1wire();
    return String(temp);
}

String getWlanIp()
{
    return networkCfg.wifiDhcp ? WiFi.localIP().toString() : networkCfg.wifiIp.toString();
}

String getWlanSsid()
{
    String ssid = WiFi.SSID();
    if (ssid.isEmpty())
    {
        ssid = "-";
    }
    return ssid;
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
    String fw;
    if (CCTool.chip.fwRev > 0)
    {
        fw = CCTool.chip.fwRev;
    }
    else
    {
        fw = systemCfg.zbFw;
    }
    return fw;
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

/*
String getRole()
{
    String role;
    switch (systemCfg.zbRole)
    {
    case COORDINATOR:
        role = "Coordinator";
        break;
    case ROUTER:
        role = "Router";
        break;
    case OPENTHREAD:
        role = "Thread";
        break;
    default:
        role = "Unknown";
        break;
    }
    return role;
}
*/

String getRole()
{
    String role = getRadioRoleKey();
    if (role.length() > 0)
    {
        role[0] = role[0] == ' ' ? role[0] : role[0] - 32;
    }
    return role;
}

const char *rst_esp = "rst_esp";
const char *rst_zig = "rst_zig";
const char *enbl_bsl = "enbl_bsl";
const char *io = "/io/";
const char *cmd = "/cmd";
const char *upd_esp = "upd_esp";
const char *upd_zb = "upd_zb";
const char *upd_esp_slash = "/upd/esp";
const char *upd_zb_slash = "/upd/zb";
const char *mdiPrefix = "mdi:";
const char *coolant_temperature = "coolant-temperature";
const char *access_point = "access-point";
const char *temperature = "temperature";
const char *diagnostic = "diagnostic";
const char *config = "config";
const char *restart = "restart";

mqttTopicsConfig mqttTopicsConfigs[] = {
    {.name = "Restart ESP",
     .sensorType = haButton,
     .sensorId = rst_esp,
     .stateTopic = String(io) + rst_esp,
     .commandTopic = cmd,
     .icon = String(mdiPrefix) + "restore-alert",
     .payloadPress = rst_esp,
     .deviceClass = restart},
    {.name = "Restart Zigbee",
     .sensorType = haButton,
     .sensorId = rst_zig,
     .stateTopic = String(io) + rst_zig,
     .commandTopic = cmd,
     .icon = String(mdiPrefix) + restart,
     .payloadPress = rst_zig,
     .deviceClass = restart},
    {.name = "Enable BSL",
     .sensorType = haButton,
     .sensorId = enbl_bsl,
     .stateTopic = String(io) + enbl_bsl,
     .commandTopic = cmd,
     .icon = "mdi:flash",
     .payloadPress = enbl_bsl,
     .deviceClass = "identify"},
    {.name = "Socket",
     .sensorType = haBinarySensor,
     .sensorId = "socket",
     .stateTopic = "/io/socket",
     .deviceClass = "connectivity"},
    {.name = "Uptime",
     .sensorType = haSensor,
     .sensorId = "uptime",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + "clock",
     .getSensorValue = getUptime},
    {.name = "WLAN IP",
     .sensorType = haSensor,
     .sensorId = "wlanIp",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + "wifi-check",
     .getSensorValue = getWlanIp},
    {.name = "WLAN SSID",
     .sensorType = haSensor,
     .sensorId = "wlanSsid",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + "wifi-marker",
     .getSensorValue = getWlanSsid},
    {.name = "WLAN RSSI",
     .sensorType = haSensor,
     .sensorId = "wlanRssi",
     .stateTopic = stateTopic,
     .deviceClass = "signal_strength",
     .unitOfMeasurement = "dBm",
     .getSensorValue = getWlanRssi},
    {.name = "LAN IP",
     .sensorType = haSensor,
     .sensorId = "lanIp",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + "check-network",
     .getSensorValue = getLanIp},
    {.name = "ESP Temperature",
     .sensorType = haSensor,
     .sensorId = temperature,
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + coolant_temperature,
     .deviceClass = temperature,
     .unitOfMeasurement = "°C",
     .getSensorValue = getStrCpuTemp},
    {.name = "1W Temperature",
     .sensorType = haSensor,
     .sensorId = "temperature1w",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + coolant_temperature,
     .deviceClass = temperature,
     .unitOfMeasurement = "°C",
     .getSensorValue = getStr1wTemp},
    {.name = "Hostname",
     .sensorType = haSensor,
     .sensorId = "hostname",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + "account-network",
     .getSensorValue = getHostname},
    {.name = "Socket Connections",
     .sensorType = haSensor,
     .sensorId = "connections",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + "check-network-outline",
     .getSensorValue = getConnections},
    {.name = "Mode",
     .sensorType = haSensor,
     .sensorId = "mode",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + access_point + "-network",
     .deviceClass = "enum",
     .entityCategory = diagnostic,
     .getSensorValue = getWorkMode},
    {.name = "Radio chip",
     .sensorType = haSensor,
     .sensorId = "zbhw",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + access_point,
     .entityCategory = diagnostic,
     .getSensorValue = getZigbeeHWrev},
    {.name = "Zigbee Role",
     .sensorType = haSensor,
     .sensorId = "zbrl",
     .stateTopic = stateTopic,
     .icon = String(mdiPrefix) + access_point,
     .entityCategory = diagnostic,
     .getSensorValue = getRole},
    {.name = "ESP32 firmware",
     .sensorType = haUpdate,
     .sensorId = upd_esp,
     .stateTopic = upd_esp_slash,
     .commandTopic = cmd,
     .entityCategory = config,
     .entityPicture = haUpdateLogoUrl,
     .payloadInstall = upd_esp,
     //.releaseUrl = "zb_upd",
     .jsonAttrTemplate = haUpdateAttrTemplate,
     .jsonAttrTopic = upd_esp_slash},
    {.name = "Radio firmware",
     .sensorType = haUpdate,
     .sensorId = upd_zb,
     .stateTopic = upd_zb_slash,
     .commandTopic = cmd,
     .entityCategory = config,
     .entityPicture = haUpdateLogoUrl,
     .payloadInstall = upd_zb,
     //.releaseUrl = "zb_upd",
     .jsonAttrTemplate = haUpdateAttrTemplate,
     .jsonAttrTopic = upd_zb_slash}};

/*{.name = "Zigbee FW version",
.sensorType = haSensor,
.sensorId = "zbfw",
.stateTopic = stateTopic,
.icon = "mdi:access-point",
.valueTemplate = "{{ value_json.zbfw }}",
.getSensorValue = getZigbeeFWver},*/

void mqttConnectSetup()
{
    LOGD("mqttConnectSetup");
    if (true) // checkDNS())
    {
        if (mqttCfg.reconnectInt == 0)
        {
            mqttCfg.reconnectInt = 30;
            LOGD("Reconnect Int didn't set. So use %d seconds", mqttCfg.reconnectInt);
        }
        if (mqttCfg.updateInt == 0)
        {
            mqttCfg.updateInt = 60;
            LOGD("Update Int didn't set. So use %d seconds", mqttCfg.updateInt);
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

        const char *clientId = vars.deviceId;
        mqttClient.setClientId(clientId);

        mqttClient.setCredentials(mqttCfg.user, mqttCfg.pass);

        // String topic = String(mqttCfg.topic) + "/avty";
        // mqttClient.setWill(topic.c_str(), 1, true, "offline");

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

        connectToMqtt();
        /*if (xTimerStart(mqttReconnectTimer, 0) != pdPASS)
        {
            LOGD("Failed to start timer");
        }*/
    }
}

void mqttDisconnectCleanup()
{
    LOGD("mqttDisconnectCleanup");

    // Остановить и удалить таймеры
    if (mqttReconnectTimer != NULL)
    {
        if (xTimerStop(mqttReconnectTimer, 0) == pdPASS)
        {
            if (xTimerDelete(mqttReconnectTimer, 0) == pdPASS)
            {
                mqttReconnectTimer = NULL;
                LOGD("mqttReconnectTimer stopped and deleted");
            }
            else
            {
                LOGD("Failed to delete mqttReconnectTimer");
            }
        }
        else
        {
            LOGD("Failed to stop mqttReconnectTimer");
        }
    }

    if (mqttPubStateTimer != NULL)
    {
        if (xTimerStop(mqttPubStateTimer, 0) == pdPASS)
        {
            if (xTimerDelete(mqttPubStateTimer, 0) == pdPASS)
            {
                mqttPubStateTimer = NULL;
                LOGD("mqttPubStateTimer stopped and deleted");
            }
            else
            {
                LOGD("Failed to delete mqttPubStateTimer");
            }
        }
        else
        {
            LOGD("Failed to stop mqttPubStateTimer");
        }
    }

    // Отключить MQTT-клиент
    if (mqttClient.connected())
    {
        mqttClient.disconnect();
        LOGD("MQTT client disconnected");
    }

    // Очистить любые другие ресурсы, если необходимо
    // Например, очистить буферы, закрыть соединения и т.д.
}

void connectToMqtt()
{
    if (!vars.zbFlashing)
    {
        LOGD("Connecting...");
        mqttClient.connect();
    }
    else
    {
        LOGD("only after ZB flash");
    }
}

void onMqttConnect(bool sessionPresent)
{
    LOGD("Connected to MQTT. Session present: %s", String(sessionPresent).c_str());

    mqttClient.subscribe((String(mqttCfg.topic) + "/cmd").c_str(), 2);

    vars.mqttConn = true;

    if (mqttCfg.discovery)
    {
        mqttPublishDiscovery();
    }

    mqttPublishIo("rst_esp", 0);
    mqttPublishIo("rst_zig", 0);
    mqttPublishIo("enbl_bsl", 0);
    mqttPublishIo("socket", vars.connectedClients ? 1 : 0);
    // mqttPublishIo("update_esp", vars.updateEspAvail);
    // mqttPublishIo("update_zb", vars.updateZbAvail);
    mqttPublishState();

    mqttPublishUpdate("esp");
    mqttPublishUpdate("zb");

    mqttPublishAvail();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    LOGI("Disconnected from MQTT: %d", reason);

    vars.mqttConn = false;

    if (!vars.zbFlashing)
    {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    static char json[512];
    // static DynamicJsonDocument doc(512);

    String topicStr(topic);
    if (topicStr.endsWith("/cmd"))
    {
        if (len < sizeof(json))
        {
            memcpy(json, payload, len);
            json[len] = '\0';

            LOGD("json - %s", String(json));

            if (strlen(json) > 0)
            {
                executeCommand(json);
            }
            else
            {
                LOGD("empty cmnd");
            }
        }
        else
        {
            LOGW("cmnd too large");
        }
    }
    topicStr.clear();
}

void executeCommand(const char *command)
{
    LOGD("mqtt cmd - %s", String(command));
    if (strcmp(command, "rst_esp") == 0)
    {
        printLogMsg("ESP restart MQTT");
        restartDevice();
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

    if (strcmp(command, "upd_esp") == 0)
    {
        mqttPublishUpdate("esp", true);
        printLogMsg("Going to update ESP");
        // to - do
    }

    if (strcmp(command, "upd_zb") == 0)
    {
        mqttPublishUpdate("zb", true);
        printLogMsg("Going to update ESP");
        // to - do
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

void mqttPublishUpdate(const String &chip, bool instState)
{
    // String state = st ? "ON" : "OFF";
    if (mqttClient.connected())
    {
        /*
        String topic = String(mqttCfg.topic) + "/io/" + io;
        LOGD("Pub Io %s at %s", state.c_str(), topic.c_str());
        mqttClient.publish(topic.c_str(), 0, true, state.c_str());*/
        static DynamicJsonDocument jsonBuff(256);
        static String mqttBuffer;
        char verArr[25];
        const char *env = STRINGIFY(BUILD_ENV_NAME);

        const char *installed_version = "installed_version";
        const char *latest_version = "latest_version";
        const char *state = "state";

        if (chip == "esp")
        {
            sprintf(verArr, "%s (%s)", VERSION, env);
            jsonBuff[installed_version] = String(verArr);
            jsonBuff[latest_version] = String(vars.lastESPVer);
            jsonBuff[state] = int(instState);
        }
        else if (chip == "zb")
        {
            jsonBuff[installed_version] = String(getZigbeeFWver());
            jsonBuff[latest_version] = String(vars.lastZBVer);
            jsonBuff[state] = int(instState);
        }

        String topic = String(mqttCfg.topic) + "/upd/" + chip;
        LOGD("Pub upd %s at %s", chip, topic.c_str());

        serializeJson(jsonBuff, mqttBuffer);

        mqttClient.publish(topic.c_str(), 1, true, mqttBuffer.c_str());
        jsonBuff.clear();
        mqttBuffer.clear();
    }
}

void mqttPublishState()
{
    if (!vars.zbFlashing)
    {
        static DynamicJsonDocument buffJson(512);
        static String mqttBuffer;

        buffJson.clear();
        for (const auto &item : mqttTopicsConfigs)
        {
            if (item.sensorId != "temperature1w" || vars.oneWireIs)
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
        }
        String topic = mqttCfg.topic + String(stateTopic);
        serializeJson(buffJson, mqttBuffer);

        LOGD("%s", mqttBuffer.c_str());

        mqttClient.publish(topic.c_str(), 0, true, mqttBuffer.c_str());

        topic.clear();
        mqttBuffer.clear();
    }
    else
    {
        LOGD("only after ZB flash");
    }
    xTimerStart(mqttPubStateTimer, 0);
}

void mqttPublishDiscovery()
{
    static DynamicJsonDocument devInfo(256);
    static DynamicJsonDocument buffJson(2048);
    static String mqttBuffer;

    devInfo.clear();
    devInfo["ids"] = vars.deviceId;
    devInfo["name"] = systemCfg.hostname; // mqttCfg.topic;
    devInfo["mf"] = "XZG";
    devInfo["mdl"] = hwConfig.board;
    char verArr[25];
    const char *env = STRINGIFY(BUILD_ENV_NAME);
    sprintf(verArr, "%s (%s)", VERSION, env);
    devInfo["sw"] = String(verArr);

    buffJson.clear();
    for (const auto &item : mqttTopicsConfigs)
    {
        if (item.sensorId != "temperature1w" || vars.oneWireIs)
        {
            buffJson["dev"] = devInfo;
            buffJson["name"] = item.name;
            buffJson["uniq_id"] = String(vars.deviceId) + "_" + item.sensorId;
            buffJson["stat_t"] = mqttCfg.topic + item.stateTopic;
            buffJson["avty_t"] = mqttCfg.topic + String(availabilityTopic);
            if (!String(item.commandTopic).isEmpty())
            {
                buffJson["cmd_t"] = mqttCfg.topic + item.commandTopic;
            }

            if (!String(item.icon).isEmpty())
            {
                buffJson["ic"] = item.icon;
            }
            if (!String(item.payloadPress).isEmpty())
            {
                buffJson["pl_prs"] = item.payloadPress;
            }
            if (item.sensorType == haSensor)
            {
                buffJson["val_tpl"] = "{{ value_json." + item.sensorId + " }}";
            }
            if (!String(item.deviceClass).isEmpty())
            {
                buffJson["dev_cla"] = item.deviceClass;
            }
            if (!String(item.unitOfMeasurement).isEmpty())
            {
                buffJson["unit_of_meas"] = item.unitOfMeasurement;
            }
            if (!String(item.entityCategory).isEmpty())
            {
                buffJson["ent_cat"] = item.entityCategory;
            }
            if (!String(item.entityPicture).isEmpty())
            {
                buffJson["ent_pic"] = item.entityPicture;
            }
            if (!String(item.payloadInstall).isEmpty())
            {
                buffJson["pl_inst"] = item.payloadInstall;
            }
            if (!String(item.releaseUrl).isEmpty())
            {
                buffJson["rel_u"] = item.releaseUrl;
            }
            if (!String(item.jsonAttrTemplate).isEmpty())
            {
                buffJson["json_attr_tpl"] = item.jsonAttrTemplate;
            }
            if (!String(item.jsonAttrTopic).isEmpty())
            {
                buffJson["json_attr_t"] = mqttCfg.topic + item.jsonAttrTopic;
            }

            String topic = String(homeAssistant) + "/" + item.sensorType + "/" + mqttCfg.topic + "/" + item.sensorId + configTopic;
            serializeJson(buffJson, mqttBuffer);
            LOGD("%s at %s", mqttBuffer.c_str(), topic.c_str());
            mqttClient.publish(topic.c_str(), 1, true, mqttBuffer.c_str());
            buffJson.clear();
            mqttBuffer.clear();
        }
    }

    devInfo.clear();
    buffJson.clear();
    mqttBuffer.clear();
}