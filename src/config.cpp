#include <Preferences.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "config.h"
#include "etc.h"

Preferences preferences;

#include "nvs.h"
#include "nvs_flash.h"
#include "HTTPClient.h"

extern struct SysVarsStruct vars;
extern struct BrdConfigStruct hwConfig;
extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

const char *networkConfigKey = "network-config";
const char *wifiEnableKey = "wifiEnable";
const char *wifiSsidKey = "wifiSsid";
const char *wifiPasswordKey = "wifiPassword";
const char *wifiDhcpKey = "wifiDhcp";
const char *wifiAddrKey = "wifiAddr";
const char *wifiMaskKey = "wifiMask";
const char *wifiGateKey = "wifiGate";
const char *wifiDns1Key = "wifiDns1";
const char *wifiDns2Key = "wifiDns2";
const char *ethEnableKey = "ethEnable";
const char *ethDhcpKey = "ethDhcp";
const char *ethAddrKey = "ethAddr";
const char *ethMaskKey = "ethMask";
const char *ethGateKey = "ethGate";
const char *ethDns1Key = "ethDns1";
const char *ethDns2Key = "ethDns2";

const char *vpnConfigKey = "vpn-config";
const char *wgEnableKey = "wgEnable";
const char *wgLocalIPKey = "wgLocalIP";
const char *wgLocalPrivKeyKey = "wgLocalPrivKey";
const char *wgEndAddrKey = "wgEndAddr";
const char *wgEndPubKeyKey = "wgEndPubKey";
const char *wgEndPortKey = "wgEndPort";
const char *hnEnableKey = "hnEnable";
const char *hnJoinCodeKey = "hnJoinCode";
const char *hnHostNameKey = "hnHostName";
const char *hnDashUrlKey = "hnDashUrl";

const char *mqttConfigKey = "mqtt-config";
const char *enableKey = "enable";
const char *serverKey = "server";
//const char *serverIPKey = "serverIP";
const char *portKey = "port";
const char *userKey = "user";
const char *passKey = "pass";
const char *topicKey = "topic";
const char *intervalKey = "interval";
const char *updateIntKey = "updateInt";
const char *discoveryKey = "discovery";
//const char *reconnectTimeKey = "reconnectTime";
const char *reconnectIntKey = "reconnectIntKey";
//const char *heartbeatTimeKey = "heartbeatTime";

const char *systemConfigKey = "system-config";
const char *keepWebKey = "keepWeb";
const char *disableWebKey = "disableWeb";
const char *webAuthKey = "webAuth";
const char *webUserKey = "webUser";
const char *webPassKey = "webPass";
const char *fwEnabledKey = "fwEnabled";
const char *fwIpKey = "fwIp";
const char *serialSpeedKey = "serialSpeed";
const char *socketPortKey = "socketPort";
const char *tempOffsetKey = "tempOffset";
const char *disableLedUSBKey = "disableLedUSB";
const char *disableLedPwrKey = "disableLedPwr";
const char *disableLedsKey = "disableLeds";
const char *refreshLogsKey = "refreshLogs";
const char *hostnameKey = "hostname";
const char *timeZoneKey = "timeZone";
const char *ntpServ1Key = "ntpServ1";
const char *ntpServ2Key = "ntpServ2";
// const char *prevWorkModeKey = "prevWorkMode";
// const char *workModeKey = "prevWorkMode";

const char *systemVarsKey = "system-vars";
const char *hwBtnIsKey = "hwBtnIs";
const char *hwLedUsbIsKey = "hwLedUsbIs";
const char *hwLedPwrIsKey = "hwLedPwrIs";
const char *hwUartSelIsKey = "hwUartSelIs";
const char *hwZigbeeIsKey = "hwZigbeeIs";
const char *workModeKey = "workMode";
const char *connectedSocketKey = "connectedSocket";
const char *connectedClientsKey = "connectedClients";
const char *socketTimeKey = "socketTime";
const char *connectedEtherKey = "connectedEther";
const char *apStartedKey = "apStarted";
const char *wifiWebSetupInProgressKey = "wifiWebSetupInProgress";
const char *vpnWgInitKey = "vpnWgInit";
const char *vpnWgConnectKey = "vpnWgConnect";
const char *vpnWgPeerIpKey = "vpnWgPeerIp";
const char *vpnWgCheckTimeKey = "vpnWgCheckTimeKey";
const char *vpnHnInitKey = "vpnHnInit";
const char *mqttConnKey = "mqttConn";
const char *mqttReconnectTimeKey = "mqttReconnectTime";
const char *mqttHeartbeatTimeKey = "mqttHeartbeatTime";
const char *zbLedStateKey = "zbLedState";
const char *zbFlashingKey = "zbFlashing";

String tag = "NVS";

void printNVSFreeSpace()
{
    nvs_stats_t nvsStats;
    nvs_get_stats(NULL, &nvsStats);

    LOGI(tag, "Total Entries: %d, Used Entries: %d, Free Entries: %d", nvsStats.total_entries, nvsStats.used_entries, (nvsStats.total_entries - nvsStats.used_entries));
}

void eraseNVS()
{
    LOGI(tag, "Going to erase NVS. It will factory reset device.");
    int timeDelay = 10;
    for (int i = 0; i < timeDelay; i++)
    {
        LOGI(tag, "%d seconds left..", (timeDelay - i));
        delay(1000);
    }
    LOGI(tag, "Erasing NVS. It will factory reset device!");
    ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err_t ret = nvs_flash_init();
    ESP_ERROR_CHECK(ret);
    LOGI(tag, "Erase complete!");
}

void initNVS()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        LOGI(tag, "ESP_ERR_NVS_NO_FREE_PAGES || ESP_ERR_NVS_NEW_VERSION_FOUND . DOWNGRADE and BACKUP CONFIG");
        while (true)
        {
            delay(1); // stop any work
        }
        // eraseNVS();
    }
    ESP_ERROR_CHECK(ret);
}

String makeJsonConfig(const NetworkConfigStruct *networkCfg,
                      const VpnConfigStruct *vpnCfg,
                      const MqttConfigStruct *mqttCfg,
                      const SystemConfigStruct *systemCfg,
                      const SysVarsStruct *systemVars)
{
    StaticJsonDocument<2048> doc;

    if (networkCfg != nullptr)
    {
        JsonObject network = doc.createNestedObject(networkConfigKey);
        serializeNetworkConfigToJson(*networkCfg, network);
    }

    if (vpnCfg != nullptr)
    {
        JsonObject vpn = doc.createNestedObject(vpnConfigKey);
        serializeVpnConfigToJson(*vpnCfg, vpn);
    }

    if (mqttCfg != nullptr)
    {
        JsonObject mqtt = doc.createNestedObject(mqttConfigKey);
        serializeMqttConfigToJson(*mqttCfg, mqtt);
    }

    if (systemCfg != nullptr)
    {
        JsonObject system = doc.createNestedObject(systemConfigKey);
        serializeSystemConfigToJson(*systemCfg, system);
    }

    if (systemVars != nullptr)
    {
        JsonObject varsJson = doc.createNestedObject(systemVarsKey);
        serializeSysVarsToJson(*systemVars, varsJson);
    }

    String output;
    serializeJsonPretty(doc, output);
    return output;
}

void saveNetworkConfig(const NetworkConfigStruct &config)
{
    preferences.begin(networkConfigKey, false);

    preferences.putBool(wifiEnableKey, config.wifiEnable);
    preferences.putString(wifiSsidKey, config.wifiSsid);
    preferences.putString(wifiPasswordKey, config.wifiPassword);
    preferences.putBool(wifiDhcpKey, config.wifiDhcp);
    preferences.putString(wifiAddrKey, config.wifiAddr.toString());
    preferences.putString(wifiMaskKey, config.wifiMask.toString());
    preferences.putString(wifiGateKey, config.wifiGate.toString());
    preferences.putString(wifiDns1Key, config.wifiDns1.toString());
    preferences.putString(wifiDns2Key, config.wifiDns2.toString());

    preferences.putBool(ethEnableKey, config.ethEnable);
    preferences.putBool(ethDhcpKey, config.ethDhcp);
    preferences.putString(ethAddrKey, config.ethAddr.toString());
    preferences.putString(ethMaskKey, config.ethMask.toString());
    preferences.putString(ethGateKey, config.ethGate.toString());
    preferences.putString(ethDns1Key, config.ethDns1.toString());
    preferences.putString(ethDns2Key, config.ethDns2.toString());

    preferences.end();
}

void loadNetworkConfig(NetworkConfigStruct &config)
{
    preferences.begin(networkConfigKey, true);

    config.wifiEnable = preferences.getBool(wifiEnableKey, false);
    strlcpy(config.wifiSsid, preferences.getString(wifiSsidKey).c_str(), sizeof(config.wifiSsid));
    strlcpy(config.wifiPassword, preferences.getString(wifiPasswordKey).c_str(), sizeof(config.wifiPassword));
    config.wifiDhcp = preferences.getBool(wifiDhcpKey, true);
    config.wifiAddr.fromString(preferences.getString(wifiAddrKey));
    config.wifiMask.fromString(preferences.getString(wifiMaskKey, NETWORK_MASK));
    config.wifiGate.fromString(preferences.getString(wifiGateKey));
    config.wifiDns1.fromString(preferences.getString(wifiDns1Key, DNS_SERV_1));
    config.wifiDns2.fromString(preferences.getString(wifiDns2Key, DNS_SERV_2));

    config.ethEnable = preferences.getBool(ethEnableKey, true);
    config.ethDhcp = preferences.getBool(ethDhcpKey, true);
    config.ethAddr.fromString(preferences.getString(ethAddrKey));
    config.ethMask.fromString(preferences.getString(ethMaskKey, NETWORK_MASK));
    config.ethGate.fromString(preferences.getString(ethGateKey));
    config.ethDns1.fromString(preferences.getString(ethDns1Key, DNS_SERV_1));
    config.ethDns2.fromString(preferences.getString(ethDns2Key, DNS_SERV_2));

    preferences.end();
}

void saveVpnConfig(const VpnConfigStruct &config)
{
    preferences.begin(vpnConfigKey, false);

    preferences.putBool(wgEnableKey, config.wgEnable);
    preferences.putString(wgLocalIPKey, config.wgLocalIP.toString());
    preferences.putString(wgLocalPrivKeyKey, config.wgLocalPrivKey);
    preferences.putString(wgEndAddrKey, config.wgEndAddr);
    preferences.putString(wgEndPubKeyKey, config.wgEndPubKey);
    preferences.putInt(wgEndPortKey, config.wgEndPort);

    preferences.putBool(hnEnableKey, config.hnEnable);
    preferences.putString(hnJoinCodeKey, config.hnJoinCode);
    preferences.putString(hnHostNameKey, config.hnHostName);
    preferences.putString(hnDashUrlKey, config.hnDashUrl);

    preferences.end();
}

void loadVpnConfig(VpnConfigStruct &config)
{
    preferences.begin(vpnConfigKey, true);

    config.wgEnable = preferences.getBool(wgEnableKey, false);
    config.wgLocalIP.fromString(preferences.getString(wgLocalIPKey));
    strlcpy(config.wgLocalPrivKey, preferences.getString(wgLocalPrivKeyKey).c_str(), sizeof(config.wgLocalPrivKey));
    strlcpy(config.wgEndAddr, preferences.getString(wgEndAddrKey).c_str(), sizeof(config.wgEndAddr));
    strlcpy(config.wgEndPubKey, preferences.getString(wgEndPubKeyKey).c_str(), sizeof(config.wgEndPubKey));
    config.wgEndPort = preferences.getInt(wgEndPortKey);

    config.hnEnable = preferences.getBool(hnEnableKey, false);
    strlcpy(config.hnJoinCode, preferences.getString(hnJoinCodeKey).c_str(), sizeof(config.hnJoinCode));
    strlcpy(config.hnHostName, preferences.getString(hnHostNameKey).c_str(), sizeof(config.hnHostName));
    strlcpy(config.hnDashUrl, preferences.getString(hnDashUrlKey).c_str(), sizeof(config.hnDashUrl));

    preferences.end();
}

void saveMqttConfig(const MqttConfigStruct &config)
{
    preferences.begin(mqttConfigKey, false);

    preferences.putBool(enableKey, config.enable);
    // preferences.putBool(connectKey, config.connect);
    preferences.putString(serverKey, config.server);
    //preferences.putString(serverIPKey, config.serverIP.toString());
    preferences.putInt(portKey, config.port);
    preferences.putString(userKey, config.user);
    preferences.putString(passKey, config.pass);
    preferences.putString(topicKey, config.topic);
    // preferences.putBool(retainKey, config.retain); // If needed
    preferences.putInt(updateIntKey, config.updateInt);
    preferences.putBool(discoveryKey, config.discovery);
    preferences.putULong(reconnectIntKey, config.reconnectInt);
    //preferences.putULong(heartbeatTimeKey, config.heartbeatTime);

    preferences.end();
}

void loadMqttConfig(MqttConfigStruct &config)
{
    preferences.begin(mqttConfigKey, true);

    config.enable = preferences.getBool(enableKey, false);
    // config.connect = preferences.getBool(connect, false);
    strlcpy(config.server, preferences.getString(serverKey, "").c_str(), sizeof(config.server));
    //config.serverIP.fromString(preferences.getString(serverIPKey));
    config.port = preferences.getInt(portKey, 1883);
    strlcpy(config.user, preferences.getString(userKey, "").c_str(), sizeof(config.user));
    strlcpy(config.pass, preferences.getString(passKey, "").c_str(), sizeof(config.pass));
    strlcpy(config.topic, preferences.getString(topicKey, "").c_str(), sizeof(config.topic));
    // config.retain = preferences.getBool(retain, false); // If needed
    config.updateInt = preferences.getInt(updateIntKey, 60);
    config.discovery = preferences.getBool(discoveryKey, true);
    config.reconnectInt = preferences.getULong(reconnectIntKey, 15);
    //config.heartbeatTime = preferences.getULong(heartbeatTimeKey, 60000);

    preferences.end();
}

void saveSystemConfig(const SystemConfigStruct &config)
{
    preferences.begin(systemConfigKey, false);

    preferences.putBool(keepWebKey, config.keepWeb);
    preferences.putBool(disableWebKey, config.disableWeb);
    preferences.putBool(webAuthKey, config.webAuth);
    preferences.putString(webUserKey, config.webUser);
    preferences.putString(webPassKey, config.webPass);
    preferences.putBool(fwEnabledKey, config.fwEnabled);
    preferences.putString(fwIpKey, config.fwIp.toString());
    preferences.putInt(serialSpeedKey, config.serialSpeed);
    preferences.putInt(socketPortKey, config.socketPort);
    preferences.putInt(tempOffsetKey, config.tempOffset);
    preferences.putBool(disableLedUSBKey, config.disableLedUSB);
    preferences.putBool(disableLedPwrKey, config.disableLedPwr);
    preferences.putBool(disableLedsKey, config.disableLeds);
    preferences.putInt(refreshLogsKey, config.refreshLogs);
    preferences.putString(hostnameKey, config.hostname);
    preferences.putString(timeZoneKey, config.timeZone);
    preferences.putString(ntpServ1Key, config.ntpServ1);
    preferences.putString(ntpServ2Key, config.ntpServ2);
    // preferences.putInt(prevWorkModeKey, static_cast<int>(config.prevWorkMode));
    preferences.putInt(workModeKey, static_cast<int>(config.workMode));

    preferences.end();
}

void loadSystemConfig(SystemConfigStruct &config)
{
    preferences.begin(systemConfigKey, true);

    config.keepWeb = preferences.getBool(keepWebKey, true);
    config.disableWeb = preferences.getBool(disableWebKey, false);
    config.webAuth = preferences.getBool(webAuthKey, false);
    strlcpy(config.webUser, preferences.getString(webUserKey, "").c_str(), sizeof(config.webUser));
    strlcpy(config.webPass, preferences.getString(webPassKey, "").c_str(), sizeof(config.webPass));
    config.fwEnabled = preferences.getBool(fwEnabledKey, false);
    config.fwIp.fromString(preferences.getString(fwIpKey, "0.0.0.0"));
    config.serialSpeed = preferences.getInt(serialSpeedKey, ZB_SERIAL_SPEED);
    config.socketPort = preferences.getInt(socketPortKey, ZB_TCP_PORT);
    config.tempOffset = preferences.getInt(tempOffsetKey, 0);
    config.disableLedUSB = preferences.getBool(disableLedUSBKey, false);
    config.disableLedPwr = preferences.getBool(disableLedPwrKey, false);
    config.disableLeds = preferences.getBool(disableLedsKey, false);
    config.refreshLogs = preferences.getInt(refreshLogsKey, 1000);
    strlcpy(config.hostname, preferences.getString(hostnameKey, "XZG").c_str(), sizeof(config.hostname)); /// to do add def host name!!
    strlcpy(config.timeZone, preferences.getString(timeZoneKey, NTP_TIME_ZONE).c_str(), sizeof(config.timeZone));
    strlcpy(config.ntpServ1, preferences.getString(ntpServ1Key, NTP_SERV_1).c_str(), sizeof(config.ntpServ1));
    strlcpy(config.ntpServ2, preferences.getString(ntpServ2Key, NTP_SERV_2).c_str(), sizeof(config.ntpServ2));
    // config.prevWorkMode = static_cast<WORK_MODE_t>(preferences.getInt(prevWorkMode, WORK_MODE_NETWORK));
    // config.prevWorkMode = static_cast<WORK_MODE_t>(preferences.getInt(prevWorkModeKey, WORK_MODE_NETWORK));
    config.workMode = static_cast<WORK_MODE_t>(preferences.getInt(workModeKey, WORK_MODE_NETWORK));

    preferences.end();
}

enum API_PAGE_t : uint8_t
{
    API_PAGE_ROOT,
    API_PAGE_GENERAL,
    API_PAGE_ETHERNET,
    API_PAGE_NETWORK,
    API_PAGE_ZIGBEE,
    API_PAGE_SECURITY,
    API_PAGE_TOOLS,
    API_PAGE_ABOUT,
    API_PAGE_MQTT,
    API_PAGE_VPN
};

void updateConfiguration(WebServer &serverWeb, SystemConfigStruct &configSys, NetworkConfigStruct &configNet, VpnConfigStruct &configVpn, MqttConfigStruct &configMqtt)
{
    const char *pageId = "pageId";
    const char *on = "on";
    const char *contTypeText = "text/plain";

    if (serverWeb.hasArg(pageId))
    {
        switch (serverWeb.arg(pageId).toInt())
        {
        case API_PAGE_GENERAL:
        {
            if (serverWeb.hasArg(coordMode))
            {
                const uint8_t mode = serverWeb.arg(coordMode).toInt();
                if (mode <= 2 && mode >= 0)
                {
                    // vars.workMode = static_cast<WORK_MODE_t>(mode);
                    // if (mode == 1)
                    // wifiWebSetupInProgress = true; //to do
                    configSys.workMode = static_cast<WORK_MODE_t>(mode);
                }
            }
            if (serverWeb.hasArg(keepWebKey))
            {
                configSys.keepWeb = serverWeb.arg(keepWebKey) == on;
            }

            
            if (serverWeb.hasArg(disableLedPwrKey))
            {
                configSys.disableLedPwr = serverWeb.arg(disableLedPwrKey) == on;
            }

            
            if (serverWeb.hasArg(disableLedUSBKey))
            {
                configSys.disableLedUSB = serverWeb.arg(disableLedUSBKey) == on;
            }

            
            if (serverWeb.hasArg(refreshLogsKey))
            {
                configSys.refreshLogs = serverWeb.arg(refreshLogsKey).toInt();
            }

            const char *hostname = "hostname";
            if (serverWeb.hasArg(hostname))
            {
                // Ensure the string does not exceed 49 characters, leaving space for the null terminator
                strncpy(configSys.hostname, serverWeb.arg(hostname).c_str(), sizeof(configSys.hostname) - 1);
                configSys.hostname[sizeof(configSys.hostname) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *timeZoneName = "timeZoneName";
            if (serverWeb.hasArg(timeZoneName))
            {
                strncpy(configSys.timeZone, serverWeb.arg(timeZoneName).c_str(), sizeof(configSys.timeZone) - 1);
                configSys.timeZone[sizeof(configSys.timeZone) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *ntpServ1 = "ntpServ1";
            if (serverWeb.hasArg(ntpServ1))
            {
                strncpy(configSys.ntpServ1, serverWeb.arg(ntpServ1).c_str(), sizeof(configSys.ntpServ1) - 1);
                configSys.ntpServ1[sizeof(configSys.ntpServ1) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            const char *ntpServ2 = "ntpServ2";
            if (serverWeb.hasArg(ntpServ2))
            {
                strncpy(configSys.ntpServ2, serverWeb.arg(ntpServ2).c_str(), sizeof(configSys.ntpServ2) - 1);
                configSys.ntpServ2[sizeof(configSys.ntpServ2) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            saveSystemConfig(configSys);
        }
        break;
        case API_PAGE_NETWORK:
        {
            const char *ethEnableKey = "ethEnbl";
            configNet.ethEnable = serverWeb.hasArg(ethEnableKey) == true;

            const char *ethDhcpKey = "dhcp";
            configNet.ethDhcp = serverWeb.hasArg(ethDhcpKey) == true;

            const char *ethAddrKey = "ipAddress";
            if (serverWeb.hasArg(ethAddrKey))
            {
                configNet.ethAddr.fromString(serverWeb.arg(ethAddrKey));
            }
            const char *ethMaskKey = "ipMask";
            if (serverWeb.hasArg(ethMaskKey))
            {
                configNet.ethMask.fromString(serverWeb.arg(ethMaskKey));
            }
            const char *ethGateKey = "ipGW";
            if (serverWeb.hasArg(ethGateKey))
            {
                configNet.ethGate.fromString(serverWeb.arg(ethGateKey));
            }
            const char *ethDns1Key = "ethDns1";
            if (serverWeb.hasArg(ethDns1Key))
            {
                configNet.ethDns1.fromString(serverWeb.arg(ethDns1Key));
            }
            const char *ethDns2Key = "ethDns2";
            if (serverWeb.hasArg(ethDns2Key))
            {
                configNet.ethDns2.fromString(serverWeb.arg(ethDns2Key));
            }

            const char *wifiEnableKey = "wifiEnbl";
            configNet.wifiEnable = serverWeb.hasArg(wifiEnableKey) == true;

            const char *wifiDhcpKey = "dhcpWiFi";
            configNet.wifiDhcp = serverWeb.hasArg(wifiDhcpKey) == true;

            const char *wifiSsidKey = "WIFISSID";
            if (serverWeb.arg(wifiSsidKey))
            {
                strncpy(configNet.wifiSsid, serverWeb.arg(wifiSsidKey).c_str(), sizeof(configNet.wifiSsid) - 1);
                configNet.wifiSsid[sizeof(configNet.wifiSsid) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            const char *wifiPasswordKey = "WIFIpassword";
            if (serverWeb.arg("WIFIpassword"))
            {
                strncpy(configNet.wifiPassword, serverWeb.arg(wifiPasswordKey).c_str(), sizeof(configNet.wifiPassword) - 1);
                configNet.wifiPassword[sizeof(configNet.wifiPassword) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            const char *wifiAddrKey = "ipAddress";
            if (serverWeb.hasArg(wifiAddrKey))
            {
                configNet.wifiAddr.fromString(serverWeb.arg(wifiAddrKey));
            }
            const char *wifiMaskKey = "ipMask";
            if (serverWeb.hasArg(wifiMaskKey))
            {
                configNet.wifiMask.fromString(serverWeb.arg(wifiMaskKey));
            }
            const char *wifiGateKey = "ipGW";
            if (serverWeb.hasArg(wifiGateKey))
            {
                configNet.wifiGate.fromString(serverWeb.arg(wifiGateKey));
            }
            const char *wifiDns1Key = "wifiDns1";
            if (serverWeb.hasArg(wifiDns1Key))
            {
                configNet.wifiDns1.fromString(serverWeb.arg(wifiDns1Key));
            }
            const char *wifiDns2Key = "wifiDns2";
            if (serverWeb.hasArg(wifiDns2Key))
            {
                configNet.wifiDns2.fromString(serverWeb.arg(wifiDns2Key));
            }

            saveNetworkConfig(configNet);

            if (configNet.wifiEnable)
            {
                WiFi.persistent(false);
                if (vars.apStarted)
                {
                    WiFi.mode(WIFI_AP_STA);
                }
                else
                {
                    WiFi.mode(WIFI_STA);
                }
                WiFi.begin(configNet.wifiSsid, configNet.wifiPassword);
            }
        }
        break;
        case API_PAGE_ZIGBEE:
        {
            const char *baud = "baud";
            if (serverWeb.hasArg(baud))
            {
                configSys.serialSpeed = serverWeb.arg(baud).toInt();
            }

            if (serverWeb.hasArg(baud))
            {
                configSys.socketPort = serverWeb.arg(portKey).toInt();
            }

            saveSystemConfig(configSys);
        }
        break;
        case API_PAGE_SECURITY:
        {
            configSys.disableWeb = serverWeb.hasArg(disableWebKey) == true;

            configSys.webAuth = serverWeb.hasArg(webAuthKey) == true;

            if (serverWeb.hasArg(webUserKey))
            {
                strncpy(configSys.webUser, serverWeb.arg(webUserKey).c_str(), sizeof(configSys.webUser) - 1);
                configSys.webUser[sizeof(configSys.webUser) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(webPassKey))
            {
                strncpy(configSys.webPass, serverWeb.arg(webPassKey).c_str(), sizeof(configSys.webPass) - 1);
                configSys.webPass[sizeof(configSys.webPass) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            configSys.fwEnabled = serverWeb.hasArg(fwEnabledKey) == true;

            if (serverWeb.hasArg(fwIpKey))
            {
                configSys.fwIp.fromString(serverWeb.arg(fwIpKey));
            }

            saveSystemConfig(configSys);
        }
        break;
        case API_PAGE_MQTT:
        {
            const char *MqttEnableKey = "MqttEnable";
            configMqtt.enable = serverWeb.hasArg(MqttEnableKey) == true;

            const char *MqttServerKey = "MqttServer";
            if (serverWeb.hasArg(MqttServerKey))
            {
                strncpy(configMqtt.server, serverWeb.arg(MqttServerKey).c_str(), sizeof(configMqtt.server) - 1);
                configMqtt.server[sizeof(configMqtt.server) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *MqttPortKey = "MqttPort";
            if (serverWeb.hasArg(MqttPortKey))
            {
                configMqtt.port = serverWeb.arg(MqttPortKey).toInt();
            }

            const char *MqttUserKey = "MqttUser";
            if (serverWeb.hasArg(MqttUserKey))
            {
                strncpy(configMqtt.user, serverWeb.arg(MqttUserKey).c_str(), sizeof(configMqtt.user) - 1);
                configMqtt.user[sizeof(configMqtt.user) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *MqttPassKey = "MqttPass";
            if (serverWeb.hasArg(MqttPassKey))
            {
                strncpy(configMqtt.pass, serverWeb.arg(MqttPassKey).c_str(), sizeof(configMqtt.pass) - 1);
                configMqtt.pass[sizeof(configMqtt.pass) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *MqttTopicKey = "MqttTopic";
            if (serverWeb.hasArg(MqttTopicKey))
            {
                strncpy(configMqtt.topic, serverWeb.arg(MqttTopicKey).c_str(), sizeof(configMqtt.topic) - 1);
                configMqtt.topic[sizeof(configMqtt.topic) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *MqttIntervalKey = "MqttInterval";
            if (serverWeb.hasArg(MqttIntervalKey))
            {
                configMqtt.updateInt = serverWeb.arg(MqttIntervalKey).toInt();
            }

            const char *MqttDiscoveryKey = "MqttDiscovery";
            configMqtt.discovery = serverWeb.hasArg(MqttDiscoveryKey) == true;

            saveMqttConfig(configMqtt);
        }
        break;
        case API_PAGE_VPN:
        {
            const char *WgEnableKey = "WgEnable";
            configVpn.wgEnable = serverWeb.hasArg(WgEnableKey) == true;

            const char *WgLocalAddrKey = "WgLocalAddr";
            if (serverWeb.hasArg(WgLocalAddrKey))
            {
                configVpn.wgLocalIP.fromString(serverWeb.arg(WgLocalAddrKey));
            }

            const char *WgLocalPrivKeyKey = "WgLocalPrivKey";
            if (serverWeb.hasArg(WgLocalPrivKeyKey))
            {
                strncpy(configVpn.wgLocalPrivKey, serverWeb.arg(WgLocalPrivKeyKey).c_str(), sizeof(configVpn.wgLocalPrivKey) - 1);
                configVpn.wgLocalPrivKey[sizeof(configVpn.wgLocalPrivKey) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *WgEndAddrKey = "WgEndAddr";
            if (serverWeb.hasArg(WgEndAddrKey))
            {
                strncpy(configVpn.wgEndAddr, serverWeb.arg(WgEndAddrKey).c_str(), sizeof(configVpn.wgEndAddr) - 1);
                configVpn.wgEndAddr[sizeof(configVpn.wgEndAddr) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *WgEndPubKeyKey = "WgEndPubKey";
            if (serverWeb.hasArg(WgEndPubKeyKey))
            {
                strncpy(configVpn.wgEndPubKey, serverWeb.arg(WgEndPubKeyKey).c_str(), sizeof(configVpn.wgEndPubKey) - 1);
                configVpn.wgEndPubKey[sizeof(configVpn.wgEndPubKey) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *WgEndPortKey = "WgEndPort";
            if (serverWeb.hasArg(WgEndPortKey))
            {
                configVpn.wgEndPort = serverWeb.arg(WgEndPortKey).toInt();
            }

            const char *HnEnableKey = "HnEnable";
            configVpn.hnEnable = serverWeb.hasArg(HnEnableKey) == true;

            const char *HnJoinCodeKey = "HnJoinCode";
            if (serverWeb.hasArg(HnJoinCodeKey))
            {
                strncpy(configVpn.hnJoinCode, serverWeb.arg(HnJoinCodeKey).c_str(), sizeof(configVpn.hnJoinCode) - 1);
                configVpn.hnJoinCode[sizeof(configVpn.hnJoinCode) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *HnHostNameKey = "HnHostName";
            if (serverWeb.hasArg(HnHostNameKey))
            {
                strncpy(configVpn.hnHostName, serverWeb.arg(HnHostNameKey).c_str(), sizeof(configVpn.hnHostName) - 1);
                configVpn.hnHostName[sizeof(configVpn.hnHostName) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            const char *HnDashUrlKey = "HnDashUrl";
            if (serverWeb.hasArg(HnDashUrlKey))
            {
                strncpy(configVpn.hnDashUrl, serverWeb.arg(HnDashUrlKey).c_str(), sizeof(configVpn.hnDashUrl) - 1);
                configVpn.hnDashUrl[sizeof(configVpn.hnDashUrl) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            saveVpnConfig(configVpn);
        }
        break;
        }
        // String cfg = makeJsonConfig(&configNet, &configVpn, &configMqtt, &configSys);
        // DEBUG_PRINTLN(cfg);
        serverWeb.send(HTTP_CODE_OK, contTypeText, "ok");
    }
    else
    {
        serverWeb.send(500, contTypeText, "bad args");
    }
}

// Serialization NetworkConfigStruct into JSON
void serializeNetworkConfigToJson(const NetworkConfigStruct &config, JsonObject obj)
{
    obj[wifiEnableKey] = config.wifiEnable;
    obj[wifiSsidKey] = config.wifiSsid;
    obj[wifiPasswordKey] = config.wifiPassword;
    obj[wifiDhcpKey] = config.wifiDhcp;
    obj[wifiAddrKey] = config.wifiAddr.toString();
    obj[wifiMaskKey] = config.wifiMask.toString();
    obj[wifiGateKey] = config.wifiGate.toString();
    obj[wifiDns1Key] = config.wifiDns1.toString();
    obj[wifiDns2Key] = config.wifiDns2.toString();
    obj[ethEnableKey] = config.ethEnable;
    obj[ethDhcpKey] = config.ethDhcp;
    obj[ethAddrKey] = config.ethAddr.toString();
    obj[ethMaskKey] = config.ethMask.toString();
    obj[ethGateKey] = config.ethGate.toString();
    obj[ethDns1Key] = config.ethDns1.toString();
    obj[ethDns2Key] = config.ethDns2.toString();
}

// Serialization VpnConfigStruct into JSON
void serializeVpnConfigToJson(const VpnConfigStruct &config, JsonObject obj)
{
    obj[wgEnableKey] = config.wgEnable;
    obj[wgLocalIPKey] = config.wgLocalIP.toString();
    obj[wgLocalPrivKeyKey] = config.wgLocalPrivKey;
    obj[wgEndAddrKey] = config.wgEndAddr;
    obj[wgEndPubKeyKey] = config.wgEndPubKey;
    obj[wgEndPortKey] = config.wgEndPort;
    obj[hnEnableKey] = config.hnEnable;
    obj[hnJoinCodeKey] = config.hnJoinCode;
    obj[hnHostNameKey] = config.hnHostName;
    obj[hnDashUrlKey] = config.hnDashUrl;
}

// Serialization MqttConfigStruct into JSON
void serializeMqttConfigToJson(const MqttConfigStruct &config, JsonObject obj)
{
    obj[enableKey] = config.enable;
    obj[serverKey] = config.server;
    //obj[serverIPKey] = config.serverIP.toString();
    obj[portKey] = config.port;
    obj[userKey] = config.user;
    obj[passKey] = config.pass;
    obj[topicKey] = config.topic;
    obj[updateIntKey] = config.updateInt;
    obj[discoveryKey] = config.discovery;
    obj[reconnectIntKey] = config.reconnectInt;
    //obj[heartbeatTimeKey] = config.heartbeatTime;
}

// Serialization SystemConfigStruct into JSON
void serializeSystemConfigToJson(const SystemConfigStruct &config, JsonObject obj)
{
    obj[keepWebKey] = config.keepWeb;
    obj[disableWebKey] = config.disableWeb;
    obj[webAuthKey] = config.webAuth;
    obj[webUserKey] = config.webUser;
    obj[webPassKey] = config.webPass;
    obj[fwEnabledKey] = config.fwEnabled;
    obj[fwIpKey] = config.fwIp.toString();
    obj[serialSpeedKey] = config.serialSpeed;
    obj[socketPortKey] = config.socketPort;
    obj[tempOffsetKey] = config.tempOffset;
    obj[disableLedUSBKey] = config.disableLedUSB;
    obj[disableLedPwrKey] = config.disableLedPwr;
    obj[disableLedsKey] = config.disableLeds;
    obj[refreshLogsKey] = config.refreshLogs;
    obj[hostnameKey] = config.hostname;
    obj[timeZoneKey] = config.timeZone;
    obj[ntpServ1Key] = config.ntpServ1;
    obj[ntpServ2Key] = config.ntpServ2;
    // obj[prevWorkModeKey] = static_cast<int>(config.prevWorkMode);
    obj[workModeKey] = static_cast<int>(config.workMode);
}

void serializeSysVarsToJson(const SysVarsStruct &vars, JsonObject obj)
{
    // Serializing system variables to JSON
    obj[hwBtnIsKey] = vars.hwBtnIs;
    obj[hwLedUsbIsKey] = vars.hwLedUsbIs;
    obj[hwLedPwrIsKey] = vars.hwLedPwrIs;
    obj[hwUartSelIsKey] = vars.hwUartSelIs;
    obj[hwZigbeeIsKey] = vars.hwZigbeeIs;

    // Assuming WORK_MODE_t can be directly cast to int for serialization
    obj[workModeKey] = static_cast<int>(vars.workMode);

    // Serializing an array of connectedSocket
    /*JsonArray connectedSocketArray = obj.createNestedArray(connectedSocket);
    for (bool socket : vars.connectedSocket) {
        connectedSocketArray.add(socket);
    }*/

    obj[connectedClientsKey] = vars.connectedClients;
    obj[socketTimeKey] = vars.socketTime;
    obj[connectedEtherKey] = vars.connectedEther;
    obj[apStartedKey] = vars.apStarted;
    obj[wifiWebSetupInProgressKey] = vars.wifiWebSetupInProgress;

    obj[vpnWgInitKey] = vars.vpnWgInit;
    obj[vpnWgConnectKey] = vars.vpnWgConnect;
    obj[vpnWgPeerIpKey] = vars.vpnWgPeerIp.toString(); // Assuming IPAddress can be converted to string
    obj[vpnWgCheckTimeKey] = vars.vpnWgCheckTime;
    obj[vpnHnInitKey] = vars.vpnHnInit;

    obj[mqttConnKey] = vars.mqttConn;
    obj[mqttReconnectTimeKey] = vars.mqttReconnectTime;
    obj[mqttHeartbeatTimeKey] = vars.mqttHeartbeatTime;

    obj[zbLedStateKey] = vars.zbLedState;
    obj[zbFlashingKey] = vars.zbFlashing;
}

bool loadFileConfigHW()
{
    const char *board = "board";
    const char *addr = "addr";
    const char *pwrPin = "pwrPin";
    const char *mdcPin = "mdcPin";
    const char *mdiPin = "mdiPin";
    const char *phyType = "phyType";
    const char *clkMode = "clkMode";
    const char *pwrAltPin = "pwrAltPin";
    const char *btnPin = "btnPin";
    const char *uartChPin = "uartChPin";
    const char *ledUsbPin = "ledUsbPin";
    const char *ledPwrPin = "ledPwrPin";
    const char *zbTxPin = "zbTxPin";
    const char *zbRxPin = "zbRxPin";
    const char *zbRstPin = "zbRstPin";
    const char *zbBslPin = "zbBslPin";

    File configFile = LittleFS.open(configFileHw, FILE_READ);

    if (!configFile)
    {
        DynamicJsonDocument config(300);
        config[board] = "";
        /*
        config[addr] = -1;
        config[pwrPin] = -1;
        config[mdcPin] = -1;
        config[mdiPin] = -1;
        config[phyType] = -1;
        config[clkMode] = -1;
        config[pwrAltPin] = -1;
        config[btnPin] = -1;
        config[uartChPin] = -1;
        config[ledUsbPin] = -1;
        config[ledPwrPin] = -1;
        config[zbTxPin] = -1;
        config[zbRxPin] = -1;
        config[zbRstPin] = -1;
        config[zbBslPin] = -1;
        */
        writeDefaultConfig(configFileHw, config);
        configFile = LittleFS.open(configFileHw, FILE_READ);
    }

    DynamicJsonDocument config(1024);
    deserializeJson(config, configFile);

    DEBUG_PRINTLN(configFile.readString());
    configFile.close();

    strlcpy(hwConfig.board, config[board] | "", sizeof(hwConfig.board));
    hwConfig.eth.addr = config[addr];
    hwConfig.eth.pwrPin = config[pwrPin];
    hwConfig.eth.mdcPin = config[mdcPin];
    hwConfig.eth.mdiPin = config[mdiPin];
    hwConfig.eth.phyType = config[phyType];
    hwConfig.eth.clkMode = config[clkMode];
    hwConfig.eth.pwrAltPin = config[pwrAltPin];
    hwConfig.mist.btnPin = config[btnPin];
    hwConfig.mist.uartSelPin = config[uartChPin];
    hwConfig.mist.ledUsbPin = config[ledUsbPin];
    hwConfig.mist.ledPwrPin = config[ledPwrPin];
    hwConfig.zb.txPin = config[zbTxPin];
    hwConfig.zb.rxPin = config[zbRxPin];
    hwConfig.zb.rstPin = config[zbRstPin];
    hwConfig.zb.bslPin = config[zbBslPin];

    if (hwConfig.board[0] != '\0' && strlen(hwConfig.board) > 0)
    {
        DEBUG_PRINTLN("hwConfig LOAD - OK");

        delay(1000);
        return true;
    }
    else
    {
        DEBUG_PRINTLN("hwConfig LOAD - ERROR");
        delay(1000);
        int searchId = 0;
        if (config["searchId"])
        {
            searchId = config["searchId"];
        }
        BrdConfigStruct *newConfig = findBrdConfig(searchId);
        if (newConfig)
        {
            DEBUG_PRINTLN(F("Saving HW config"));

            DynamicJsonDocument config(512);
            config[board] = newConfig->board;
            config[addr] = newConfig->eth.addr;
            config[pwrPin] = newConfig->eth.pwrPin;
            config[mdcPin] = newConfig->eth.mdcPin;
            config[mdiPin] = newConfig->eth.mdiPin;
            config[phyType] = newConfig->eth.phyType;
            config[clkMode] = newConfig->eth.clkMode;
            config[pwrAltPin] = newConfig->eth.pwrAltPin;
            config[btnPin] = newConfig->mist.btnPin;
            config[uartChPin] = newConfig->mist.uartSelPin;
            config[ledUsbPin] = newConfig->mist.ledUsbPin;
            config[ledPwrPin] = newConfig->mist.ledPwrPin;
            config[zbTxPin] = newConfig->zb.txPin;
            config[zbRxPin] = newConfig->zb.rxPin;
            config[zbRstPin] = newConfig->zb.rstPin;
            config[zbBslPin] = newConfig->zb.bslPin;
            writeDefaultConfig(configFileHw, config);
            // configFile = LittleFS.open(configFileHw, FILE_WRITE);
            // serializeJson(config, configFile);

            // serializeJson(config, Serial);
            // configFile.close();

            // delay(500);
            DEBUG_PRINTLN(F("Restarting..."));
            ESP.restart();
        }
    }
    return false;
}

/* Previous firmware read config support. start */

const char *msg_file_rm = "OK. Remove old format file";
const char *msg_open_f = "open failed";

bool loadFileSystemVar()
{
    DEBUG_PRINT(F(configFileSystem));

    File configFile = LittleFS.open(configFileSystem, FILE_READ);
    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        return false;
    }

    systemCfg.tempOffset = (int)doc[tempOffsetKey];

    configFile.close();
    DEBUG_PRINTLN(F(msg_file_rm));
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileSystem);
    return true;
}

bool loadFileConfigWifi()
{
    DEBUG_PRINT(F(configFileWifi));

    const char *enableWiFi = "enableWiFi";
    const char *ssid = "ssid";
    const char *dhcpWiFi = "dhcpWiFi";
    const char *ip = "ip";
    const char *mask = "mask";
    const char *gw = "gw";

    File configFile = LittleFS.open(configFileWifi, FILE_READ);
    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        return false;
    }

    networkCfg.wifiDhcp = (int)doc[dhcpWiFi];
    strlcpy(networkCfg.wifiSsid, doc[ssid] | "", sizeof(networkCfg.wifiSsid));
    strlcpy(networkCfg.wifiPassword, doc[passKey] | "", sizeof(networkCfg.wifiPassword));

    networkCfg.wifiAddr.fromString(doc[ip] | "");
    networkCfg.wifiMask.fromString(doc[mask] | "");
    networkCfg.wifiGate.fromString(doc[gw] | "");

    configFile.close();
    DEBUG_PRINTLN(F(msg_file_rm));
    saveNetworkConfig(networkCfg);
    LittleFS.remove(configFileWifi);
    return true;
}

bool loadFileConfigEther()
{
    DEBUG_PRINT(F(configFileEther));

    const char *dhcp = "dhcp";
    const char *ip = "ip";
    const char *mask = "mask";
    const char *gw = "gw";
    File configFile = LittleFS.open(configFileEther, FILE_READ);
    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        return false;
    }

    networkCfg.ethDhcp = (int)doc[dhcp];
    networkCfg.ethAddr.fromString(doc[ip] | "");
    networkCfg.ethMask.fromString(doc[mask] | "");
    networkCfg.ethGate.fromString(doc[gw] | "");

    configFile.close();
    DEBUG_PRINTLN(F(msg_file_rm));
    saveNetworkConfig(networkCfg);
    LittleFS.remove(configFileEther);
    return true;
}

bool loadFileConfigGeneral()
{
    DEBUG_PRINT(F(configFileGeneral));
    const char *hostname = "hostname";
    
    
    File configFile = LittleFS.open(configFileGeneral, FILE_READ);

    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        return false;
    }

    if ((double)doc[refreshLogsKey] < 1000)
    {
        systemCfg.refreshLogs = 1000;
    }
    else
    {
        systemCfg.refreshLogs = (int)doc[refreshLogsKey];
    }
    // DEBUG_PRINT(F("[loadFileConfigGeneral] 'doc[coordMode]' res is: "));
    // DEBUG_PRINTLN(String((uint8_t)doc[coordMode]));

    strlcpy(systemCfg.hostname, doc[hostname] | "", sizeof(systemCfg.hostname));

    vars.workMode = static_cast<WORK_MODE_t>((uint8_t)doc[coordMode]);
    systemCfg.prevWorkMode = static_cast<WORK_MODE_t>((uint8_t)doc[prevCoordMode]);
    // DEBUG_PRINT(F("[loadFileConfigGeneral] 'vars.workMode' res is: "));
    // DEBUG_PRINTLN(String(vars.workMode));

    systemCfg.disableLedPwr = (uint8_t)doc[disableLedPwrKey];
    // DEBUG_PRINTLN(F("[loadFileConfigGeneral] disableLedPwr"));
    systemCfg.disableLedUSB = (uint8_t)doc[disableLedUSBKey];
    // DEBUG_PRINTLN(F("[loadFileConfigGeneral] disableLedUSB"));
    systemCfg.disableLeds = (uint8_t)doc[disableLedsKey];
    // DEBUG_PRINTLN(F("[loadFileConfigGeneral] disableLeds"));
    systemCfg.keepWeb = (uint8_t)doc[keepWebKey];
    // DEBUG_PRINTLN(F("[loadFileConfigGeneral] disableLeds"));
    strlcpy(systemCfg.timeZone, doc[timeZoneKey] | "", sizeof(systemCfg.timeZone));

    configFile.close();
    DEBUG_PRINTLN(F(msg_file_rm));
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileGeneral);
    return true;
}

bool loadFileConfigSecurity()
{
    DEBUG_PRINT(F(configFileSecurity));
    File configFile = LittleFS.open(configFileSecurity, FILE_READ);
    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        return false;
    }

    systemCfg.disableWeb = (uint8_t)doc[disableWebKey];
    systemCfg.webAuth = (uint8_t)doc[webAuthKey];
    strlcpy(systemCfg.webUser, doc[webUserKey] | "", sizeof(systemCfg.webUser));
    strlcpy(systemCfg.webPass, doc[webPassKey] | "", sizeof(systemCfg.webPass));
    systemCfg.fwEnabled = (uint8_t)doc[fwEnabledKey];
    systemCfg.fwIp.fromString(doc[fwIpKey] | "0.0.0.0");

    configFile.close();
    DEBUG_PRINTLN(F(msg_file_rm));
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileSecurity);
    return true;
}

bool loadFileConfigSerial()
{
    DEBUG_PRINT(F(configFileSerial));
    const char *baud = "baud";
    File configFile = LittleFS.open(configFileSerial, FILE_READ);

    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    serializeJson(doc, Serial);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        return false;
    }

    systemCfg.serialSpeed = (int)doc[baud];
    systemCfg.socketPort = (int)doc[portKey];

    configFile.close();
    DEBUG_PRINTLN(F(msg_file_rm));
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileSerial);
    return true;
}

bool loadFileConfigMqtt()
{
    DEBUG_PRINT(F(configFileMqtt));

    
    File configFile = LittleFS.open(configFileMqtt, FILE_READ);
    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    serializeJson(doc, Serial);
    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        if (error == DeserializationError::EmptyInput)
        {
            DEBUG_PRINTLN(F(msg_file_rm));
            LittleFS.remove(configFileMqtt);
        }
        return false;
    }

    mqttCfg.enable = (int)doc[enableKey];
    strlcpy(mqttCfg.server, doc[serverKey] | "", sizeof(mqttCfg.server));
    //mqttCfg.serverIP.fromString(mqttCfg.server); // parse_ip_address(mqttCfg.server);
    mqttCfg.port = (int)doc[portKey];
    strlcpy(mqttCfg.user, doc[userKey] | "", sizeof(mqttCfg.user));
    strlcpy(mqttCfg.pass, doc[passKey] | "", sizeof(mqttCfg.pass));
    strlcpy(mqttCfg.topic, doc[topicKey] | "", sizeof(mqttCfg.topic));
    mqttCfg.updateInt = (int)doc[intervalKey];
    mqttCfg.discovery = (int)doc[discoveryKey];

    configFile.close();
    saveMqttConfig(mqttCfg);
    DEBUG_PRINTLN(F("OK"));
    return true;
}

bool loadFileConfigWg()
{
    DEBUG_PRINT(F(configFileWg));
    const char *localAddr = "localAddr";
    const char *localPrivKey = "localIP";
    const char *endAddr = "endAddr";
    const char *endPubKey = "endPubKey";
    const char *endPort = "endPort";

    File configFile = LittleFS.open(configFileWg, FILE_READ);
    if (!configFile)
    {
        DEBUG_PRINTLN(F(msg_open_f));
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        DEBUG_PRINTLN(error.f_str());
        configFile.close();
        if (error == DeserializationError::EmptyInput)
        {
            DEBUG_PRINTLN(F(msg_file_rm));
            LittleFS.remove(configFileWg);
        }
        return false;
    }

    vpnCfg.wgEnable = (int)doc[enableKey];

    // strlcpy(WgSettings.localAddr, doc[localAddr] | "", sizeof(WgSettings.localAddr));

    vpnCfg.wgLocalIP.fromString(doc[localAddr] | ""); // parse_ip_address(doc[localAddr]);

    strlcpy(vpnCfg.wgLocalPrivKey, doc[localPrivKey] | "", sizeof(vpnCfg.wgLocalPrivKey));
    strlcpy(vpnCfg.wgEndAddr, doc[endAddr] | "", sizeof(vpnCfg.wgEndAddr));
    strlcpy(vpnCfg.wgEndPubKey, doc[endPubKey] | "", sizeof(vpnCfg.wgEndPubKey));
    vpnCfg.wgEndPort = (int)doc[endPort];

    configFile.close();
    saveVpnConfig(vpnCfg);
    DEBUG_PRINTLN(F("OK"));
    return true;
}

/* Previous firmware read config support. end */