#include <Preferences.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "HTTPClient.h"

#include "config.h"
#include "etc.h"
#include "web.h"
#include "const/keys.h"

Preferences preferences;

extern struct SysVarsStruct vars;
extern struct ThisConfigStruct hwConfig;
extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

void getNvsStats(int *total, int *used)
{
    nvs_stats_t nvsStats;
    nvs_get_stats(NULL, &nvsStats);

    *total = nvsStats.total_entries;
    *used = nvsStats.used_entries;
}

void printNVSFreeSpace()
{
    int total, used;
    getNvsStats(&total, &used);
    LOGI("Total: %d, Used: %d, Free: %d", total, used, (total - used));
}

void eraseNVS()
{
    LOGD("Going to erase NVS");
    for (uint8_t i = 0; i < TIMEOUT_FACTORY_RESET; i++)
    {
        LOGD("%d seconds left..", (TIMEOUT_FACTORY_RESET - i));
        delay(1000);
    }
    LOGD("Erasing NVS");
    ESP_ERROR_CHECK(nvs_flash_erase());
    esp_err_t ret = nvs_flash_init();
    ESP_ERROR_CHECK(ret);
    LOGD("Complete!");
}

void initNVS()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        LOGD("ESP_ERR_NVS_NO_FREE_PAGES || ESP_ERR_NVS_NEW_VERSION_FOUND . DOWNGRADE and BACKUP CONFIG");
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
                      const SysVarsStruct *systemVars,
                      const ThisConfigStruct *hwConfig)
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

    if (hwConfig != nullptr)
    {
        JsonObject hw = doc.createNestedObject(hwConfigKey);
        serializeHwConfigToJson(*hwConfig, hw);
    }

    String output;
    serializeJsonPretty(doc, output);
    return output;
}

void saveNetworkConfig(const NetworkConfigStruct &config)
{
    preferences.begin(networkConfigKey, false);

    preferences.putBool(wifiEnblKey, config.wifiEnable);
    preferences.putString(wifiSsidKey, config.wifiSsid);
    preferences.putString(wifiPassKey, config.wifiPass);
    preferences.putBool(wifiDhcpKey, config.wifiDhcp);
    preferences.putString(wifiIpKey, config.wifiIp.toString());
    preferences.putString(wifiMaskKey, config.wifiMask.toString());
    preferences.putString(wifiGateKey, config.wifiGate.toString());
    preferences.putString(wifiDns1Key, config.wifiDns1.toString());
    preferences.putString(wifiDns2Key, config.wifiDns2.toString());
    preferences.putInt(wifiPwrKey, static_cast<int>(config.wifiPower));
    preferences.putInt(wifiModeKey, config.wifiMode);

    preferences.putBool(ethEnblKey, config.ethEnable);
    preferences.putBool(ethDhcpKey, config.ethDhcp);
    preferences.putString(ethIpKey, config.ethIp.toString());
    preferences.putString(ethMaskKey, config.ethMask.toString());
    preferences.putString(ethGateKey, config.ethGate.toString());
    preferences.putString(ethDns1Key, config.ethDns1.toString());
    preferences.putString(ethDns2Key, config.ethDns2.toString());

    preferences.end();
}

void loadNetworkConfig(NetworkConfigStruct &config)
{
    preferences.begin(networkConfigKey, true);

    config.wifiEnable = preferences.getBool(wifiEnblKey, false);
    strlcpy(config.wifiSsid, preferences.getString(wifiSsidKey).c_str(), sizeof(config.wifiSsid));
    strlcpy(config.wifiPass, preferences.getString(wifiPassKey).c_str(), sizeof(config.wifiPass));
    config.wifiDhcp = preferences.getBool(wifiDhcpKey, true);
    config.wifiIp.fromString(preferences.getString(wifiIpKey));
    config.wifiMask.fromString(preferences.getString(wifiMaskKey, NETWORK_MASK));
    config.wifiGate.fromString(preferences.getString(wifiGateKey));
    config.wifiDns1.fromString(preferences.getString(wifiDns1Key, DNS_SERV_1));
    config.wifiDns2.fromString(preferences.getString(wifiDns2Key, DNS_SERV_2));

    config.wifiPower = static_cast<wifi_power_t>(preferences.getInt(wifiPwrKey, WIFI_POWER_19_5dBm));
    config.wifiMode = preferences.getInt(wifiModeKey, WIFI_PROTOCOL_11B);

    config.ethEnable = preferences.getBool(ethEnblKey, true);
    config.ethDhcp = preferences.getBool(ethDhcpKey, true);
    config.ethIp.fromString(preferences.getString(ethIpKey));
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
    preferences.putString(wgLocalSubnetKey, config.wgLocalSubnet.toString());
    preferences.putInt(wgLocalPortKey, config.wgLocalPort);
    preferences.putString(wgLocalGatewayKey, config.wgLocalGateway.toString());
    preferences.putString(wgLocalPrivKeyKey, config.wgLocalPrivKey);
    preferences.putString(wgEndAddrKey, config.wgEndAddr);
    preferences.putString(wgEndPubKeyKey, config.wgEndPubKey);
    preferences.putInt(wgEndPortKey, config.wgEndPort);
    preferences.putString(wgAllowedIPKey, config.wgAllowedIP.toString());
    preferences.putString(wgAllowedMaskKey, config.wgAllowedMask.toString());
    preferences.putBool(wgMakeDefaultKey, config.wgMakeDefault);
    preferences.putString(wgPreSharedKeyKey, config.wgPreSharedKey);

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
    config.wgLocalSubnet.fromString(preferences.getString(wgLocalSubnetKey, NETWORK_MASK));
    config.wgLocalPort = preferences.getInt(wgLocalPortKey, 33333);
    config.wgLocalGateway.fromString(preferences.getString(wgLocalGatewayKey, NETWORK_ZERO));
    strlcpy(config.wgLocalPrivKey, preferences.getString(wgLocalPrivKeyKey).c_str(), sizeof(config.wgLocalPrivKey));
    strlcpy(config.wgEndAddr, preferences.getString(wgEndAddrKey).c_str(), sizeof(config.wgEndAddr));
    strlcpy(config.wgEndPubKey, preferences.getString(wgEndPubKeyKey).c_str(), sizeof(config.wgEndPubKey));
    config.wgEndPort = preferences.getInt(wgEndPortKey);
    config.wgAllowedIP.fromString(preferences.getString(wgAllowedIPKey, NETWORK_ZERO));
    config.wgAllowedMask.fromString(preferences.getString(wgAllowedMaskKey, NETWORK_ZERO));
    config.wgMakeDefault = preferences.getBool(wgMakeDefaultKey, true);
    strlcpy(config.wgPreSharedKey, preferences.getString(wgPreSharedKeyKey).c_str(), sizeof(config.wgPreSharedKey));

    config.hnEnable = preferences.getBool(hnEnableKey, false);
    strlcpy(config.hnJoinCode, preferences.getString(hnJoinCodeKey).c_str(), sizeof(config.hnJoinCode));
    strlcpy(config.hnDashUrl, preferences.getString(hnDashUrlKey, "default").c_str(), sizeof(config.hnDashUrl));

    preferences.end();
}

void saveHwConfig(const ThisConfigStruct &config)
{
    LOGD("saveHwConfig");
    preferences.begin(hwConfigKey, false);
    /*
        char board[50];
        EthConfig eth;
        ZbConfig zb;
        MistConfig mist;
    */

    preferences.putString(boardKey, config.board);
    preferences.putInt(addrKey, config.eth.addr);
    preferences.putInt(pwrPinKey, config.eth.pwrPin);
    preferences.putInt(mdcPinKey, config.eth.mdcPin);
    preferences.putInt(mdiPinKey, config.eth.mdiPin);
    preferences.putInt(phyTypeKey, config.eth.phyType);
    preferences.putInt(clkModeKey, config.eth.clkMode);
    // preferences.putInt(pwrAltPinKey, config.eth.pwrAltPin);

    preferences.putInt(zbTxPinKey, config.zb.txPin);
    preferences.putInt(zbRxPinKey, config.zb.rxPin);
    preferences.putInt(zbRstPinKey, config.zb.rstPin);
    preferences.putInt(zbBslPinKey, config.zb.bslPin);

    preferences.putInt(btnPinKey, config.mist.btnPin);
    preferences.putInt(btnPlrKey, config.mist.btnPlr);
    preferences.putInt(uartSelPinKey, config.mist.uartSelPin);
    preferences.putInt(uartSelPlrKey, config.mist.uartSelPlr);
    preferences.putInt(ledModePinKey, config.mist.ledModePin);
    preferences.putInt(ledModePlrKey, config.mist.ledModePlr);
    preferences.putInt(ledPwrPinKey, config.mist.ledPwrPin);
    preferences.putInt(ledPwrPlrKey, config.mist.ledPwrPlr);

    preferences.end();
    LOGD("saveHwConfig end");
}

void loadHwConfig(ThisConfigStruct &config)
{
    preferences.begin(hwConfigKey, true);

    strlcpy(config.board, preferences.getString(boardKey).c_str(), sizeof(config.board));

    config.eth.addr = preferences.getInt(addrKey, 0);
    config.eth.pwrPin = preferences.getInt(pwrPinKey, 0);
    config.eth.mdcPin = preferences.getInt(mdcPinKey, -1);
    config.eth.mdiPin = preferences.getInt(mdiPinKey, -1);
    config.eth.phyType = static_cast<eth_phy_type_t>(preferences.getInt(phyTypeKey, ETH_PHY_LAN8720));
    config.eth.clkMode = static_cast<eth_clock_mode_t>(preferences.getInt(clkModeKey, ETH_CLOCK_GPIO0_IN));
    // config.eth.pwrAltPin = preferences.getInt(pwrAltPinKey, -1);

    config.zb.txPin = preferences.getInt(zbTxPinKey, -1);
    config.zb.rxPin = preferences.getInt(zbRxPinKey, -1);
    config.zb.rstPin = preferences.getInt(zbRstPinKey, -1);
    config.zb.bslPin = preferences.getInt(zbBslPinKey, -1);

    config.mist.btnPin = preferences.getInt(btnPinKey, -1);
    config.mist.btnPlr = preferences.getInt(btnPlrKey, -1);
    config.mist.uartSelPin = preferences.getInt(uartSelPinKey, -1);
    config.mist.uartSelPlr = preferences.getInt(uartSelPlrKey, -1);
    config.mist.ledModePin = preferences.getInt(ledModePinKey, -1);
    config.mist.ledModePlr = preferences.getInt(ledModePlrKey, -1);
    config.mist.ledPwrPin = preferences.getInt(ledPwrPinKey, -1);
    config.mist.ledPwrPlr = preferences.getInt(ledPwrPlrKey, -1);

    preferences.end();

    String cfg = makeJsonConfig(NULL, NULL, NULL, NULL, NULL, &config);
    LOGI("\n%s", cfg.c_str());
}

void saveMqttConfig(const MqttConfigStruct &config)
{
    preferences.begin(mqttConfigKey, false);

    preferences.putBool(enableKey, config.enable);
    // preferences.putBool(connectKey, config.connect);
    preferences.putString(serverKey, config.server);
    // preferences.putString(serverIPKey, config.serverIP.toString());
    preferences.putInt(portKey, config.port);
    preferences.putString(userKey, config.user);
    preferences.putString(passKey, config.pass);
    preferences.putString(topicKey, config.topic);
    // preferences.putBool(retainKey, config.retain); // If needed
    preferences.putInt(updateIntKey, config.updateInt);
    preferences.putBool(discoveryKey, config.discovery);
    preferences.putULong(reconnectIntKey, config.reconnectInt);
    // preferences.putULong(heartbeatTimeKey, config.heartbeatTime);

    preferences.end();
}

void loadMqttConfig(MqttConfigStruct &config)
{
    preferences.begin(mqttConfigKey, true);

    config.enable = preferences.getBool(enableKey, false);
    // config.connect = preferences.getBool(connect, false);
    strlcpy(config.server, preferences.getString(serverKey, "").c_str(), sizeof(config.server));
    // config.serverIP.fromString(preferences.getString(serverIPKey));
    config.port = preferences.getInt(portKey, 1883);
    strlcpy(config.user, preferences.getString(userKey, "").c_str(), sizeof(config.user));
    strlcpy(config.pass, preferences.getString(passKey, "").c_str(), sizeof(config.pass));
    // config.retain = preferences.getBool(retain, false); // If needed
    config.updateInt = preferences.getInt(updateIntKey, 60);
    config.discovery = preferences.getBool(discoveryKey, true);
    config.reconnectInt = preferences.getULong(reconnectIntKey, 15);
    // config.heartbeatTime = preferences.getULong(heartbeatTimeKey, 60000);

    preferences.end();
}

void saveSystemConfig(const SystemConfigStruct &config)
{
    preferences.begin(systemConfigKey, false);

    // preferences.putBool(keepWebKey, config.keepWeb);
    preferences.putBool(disableWebKey, config.disableWeb);
    preferences.putBool(webAuthKey, config.webAuth);
    preferences.putString(webUserKey, config.webUser);
    preferences.putString(webPassKey, config.webPass);
    preferences.putBool(fwEnabledKey, config.fwEnabled);
    preferences.putString(fwIpKey, config.fwIp.toString());
    preferences.putString(fwMaskKey, config.fwMask.toString());
    preferences.putInt(serialSpeedKey, config.serialSpeed);
    preferences.putInt(socketPortKey, config.socketPort);
    preferences.putInt(tempOffsetKey, config.tempOffset);
    preferences.putBool(disableLedUSBKey, config.disableLedUSB);
    preferences.putBool(disableLedPwrKey, config.disableLedPwr);
    preferences.putInt(refreshLogsKey, config.refreshLogs);
    preferences.putString(hostnameKey, config.hostname);
    preferences.putString(timeZoneKey, config.timeZone);
    preferences.putString(ntpServ1Key, config.ntpServ1);
    preferences.putString(ntpServ2Key, config.ntpServ2);
    preferences.putBool(nmEnableKey, config.nmEnable);
    preferences.putString(nmStartHourKey, config.nmStart);
    preferences.putString(nmEndHourKey, config.nmEnd);
    // preferences.putInt(prevWorkModeKey, static_cast<int>(config.prevWorkMode));
    preferences.putInt(workModeKey, static_cast<int>(config.workMode));

    preferences.putInt(zbRoleKey, static_cast<int>(config.zbRole));
    preferences.putString(zbFwKey, config.zbFw);

    preferences.putString(updCheckTimeKey, config.updCheckTime);
    preferences.putString(updCheckDayKey, config.updCheckDay);
    preferences.putBool(updAutoInstKey, config.updAutoInst);

    preferences.end();
}


void loadSystemConfig(SystemConfigStruct &config)
{
    preferences.begin(systemConfigKey, true);

    // config.keepWeb = preferences.getBool(keepWebKey, true);
    config.disableWeb = preferences.getBool(disableWebKey, false);
    config.webAuth = preferences.getBool(webAuthKey, false);
    strlcpy(config.webUser, preferences.getString(webUserKey, "").c_str(), sizeof(config.webUser));
    strlcpy(config.webPass, preferences.getString(webPassKey, "").c_str(), sizeof(config.webPass));
    config.fwEnabled = preferences.getBool(fwEnabledKey, false);
    config.fwIp.fromString(preferences.getString(fwIpKey, "0.0.0.0"));
    config.fwMask.fromString(preferences.getString(fwMaskKey, "0.0.0.0"));
    config.serialSpeed = preferences.getInt(serialSpeedKey, ZB_SERIAL_SPEED);
    config.socketPort = preferences.getInt(socketPortKey, ZB_TCP_PORT);
    config.tempOffset = preferences.getInt(tempOffsetKey, 0);
    config.disableLedUSB = preferences.getBool(disableLedUSBKey, false);
    config.disableLedPwr = preferences.getBool(disableLedPwrKey, false);
    config.refreshLogs = preferences.getInt(refreshLogsKey, 2);
    strlcpy(config.timeZone, preferences.getString(timeZoneKey, NTP_TIME_ZONE).c_str(), sizeof(config.timeZone));
    strlcpy(config.ntpServ1, preferences.getString(ntpServ1Key, NTP_SERV_1).c_str(), sizeof(config.ntpServ1));
    strlcpy(config.ntpServ2, preferences.getString(ntpServ2Key, NTP_SERV_2).c_str(), sizeof(config.ntpServ2));

    config.nmEnable = preferences.getBool(nmEnableKey, false);
    strlcpy(config.nmStart, preferences.getString(nmStartHourKey, NM_START_TIME).c_str(), sizeof(config.nmStart));
    strlcpy(config.nmEnd, preferences.getString(nmEndHourKey, NM_END_TIME).c_str(), sizeof(config.nmEnd));
    // config.prevWorkMode = static_cast<WORK_MODE_t>(preferences.getInt(prevWorkMode, WORK_MODE_NETWORK));
    // config.prevWorkMode = static_cast<WORK_MODE_t>(preferences.getInt(prevWorkModeKey, WORK_MODE_NETWORK));
    config.workMode = static_cast<WORK_MODE_t>(preferences.getInt(workModeKey, WORK_MODE_NETWORK));

    config.zbRole = static_cast<ZB_ROLE_t>(preferences.getInt(zbRoleKey, UNDEFINED));
    strlcpy(config.zbFw, preferences.getString(zbFwKey, "?").c_str(), sizeof(config.zbFw));

    strlcpy(config.updCheckTime, preferences.getString(updCheckTimeKey, UPD_CHK_TIME).c_str(), sizeof(config.updCheckTime));
    strlcpy(config.updCheckDay, preferences.getString(updCheckDayKey, UPD_CHK_DAY).c_str(), sizeof(config.updCheckDay));
    config.updAutoInst = preferences.getBool(updAutoInstKey, false);

    preferences.end();
}

void writeDeviceId(SystemConfigStruct &sysConfig, VpnConfigStruct &vpnConfig, MqttConfigStruct &mqttConfig)
{
    preferences.begin(systemConfigKey, true);
    strlcpy(sysConfig.hostname, preferences.getString(hostnameKey, String(vars.deviceId)).c_str(), sizeof(sysConfig.hostname));
    preferences.end();
    preferences.begin(vpnConfigKey, true);
    strlcpy(vpnConfig.hnHostName, preferences.getString(hnHostNameKey, String(vars.deviceId)).c_str(), sizeof(vpnConfig.hnHostName));
    preferences.end();
    preferences.begin(mqttConfigKey, true);
    strlcpy(mqttConfig.topic, preferences.getString(topicKey, String(vars.deviceId)).c_str(), sizeof(mqttConfig.topic));
    preferences.end();
    LOGD("Sysconfig hostname: %s", sysConfig.hostname);
}

/*
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
*/

void updateConfiguration(WebServer &serverWeb, SystemConfigStruct &configSys, NetworkConfigStruct &configNet, VpnConfigStruct &configVpn, MqttConfigStruct &configMqtt)
{
    const char *pageId = "pageId";
    const char *on = "on";

    if (serverWeb.hasArg(pageId))
    {
        switch (serverWeb.arg(pageId).toInt())
        {
        case API_PAGE_GENERAL:
        {
            /*if (serverWeb.hasArg(coordMode))
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

            configSys.keepWeb = serverWeb.hasArg(keepWebKey) == true;*/

            configSys.disableLedPwr = serverWeb.hasArg(disableLedPwrKey) == true;

            configSys.disableLedUSB = serverWeb.hasArg(disableLedUSBKey) == true;

            if (serverWeb.hasArg(refreshLogsKey))
            {
                configSys.refreshLogs = serverWeb.arg(refreshLogsKey).toInt();
            }

            if (serverWeb.hasArg(hostnameKey))
            {
                // Ensure the string does not exceed 49 characters, leaving space for the null terminator
                strncpy(configSys.hostname, serverWeb.arg(hostnameKey).c_str(), sizeof(configSys.hostname) - 1);
                configSys.hostname[sizeof(configSys.hostname) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(timeZoneNameKey))
            {
                strncpy(configSys.timeZone, serverWeb.arg(timeZoneNameKey).c_str(), sizeof(configSys.timeZone) - 1);
                configSys.timeZone[sizeof(configSys.timeZone) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(ntpServ1Key))
            {
                strncpy(configSys.ntpServ1, serverWeb.arg(ntpServ1Key).c_str(), sizeof(configSys.ntpServ1) - 1);
                configSys.ntpServ1[sizeof(configSys.ntpServ1) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(ntpServ2Key))
            {
                strncpy(configSys.ntpServ2, serverWeb.arg(ntpServ2Key).c_str(), sizeof(configSys.ntpServ2) - 1);
                configSys.ntpServ2[sizeof(configSys.ntpServ2) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            configSys.nmEnable = serverWeb.hasArg(nmEnableKey) == true;

            if (serverWeb.hasArg(nmStartHourKey))
            {
                // LOGD("nmStartHourKey %s", String(serverWeb.arg(nmStartHourKey)));
                // Serial.println(convertTimeToCron(serverWeb.arg(nmStartHourKey)));
                strncpy(configSys.nmStart, serverWeb.arg(nmStartHourKey).c_str(), sizeof(configSys.nmStart) - 1);
                configSys.nmStart[sizeof(configSys.nmStart) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            if (serverWeb.hasArg(nmEndHourKey))
            {
                // LOGD("nmEndHourKey %s", String(serverWeb.arg(nmEndHourKey)));
                // Serial.println(convertTimeToCron(serverWeb.arg(nmEndHourKey)));
                strncpy(configSys.nmEnd, serverWeb.arg(nmEndHourKey).c_str(), sizeof(configSys.nmEnd) - 1);
                configSys.nmEnd[sizeof(configSys.nmEnd) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(updCheckTimeKey))
            {
                strncpy(configSys.updCheckTime, serverWeb.arg(updCheckTimeKey).c_str(), sizeof(configSys.updCheckTime) - 1);
                configSys.updCheckTime[sizeof(configSys.updCheckTime) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(updCheckDayKey))
            {
                strncpy(configSys.updCheckDay, serverWeb.arg(updCheckDayKey).c_str(), sizeof(configSys.updCheckDay) - 1);
                configSys.updCheckDay[sizeof(configSys.updCheckDay) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            configSys.updAutoInst = serverWeb.hasArg(updAutoInstKey) == true;

            saveSystemConfig(configSys);
        }
        break;
        case API_PAGE_NETWORK:
        {
            configNet.ethEnable = serverWeb.hasArg(ethEnblKey) == true;

            configNet.ethDhcp = serverWeb.hasArg(ethDhcpKey) == true;

            if (serverWeb.hasArg(ethIpKey))
            {
                configNet.ethIp.fromString(serverWeb.arg(ethIpKey));
            }

            if (serverWeb.hasArg(ethMaskKey))
            {
                configNet.ethMask.fromString(serverWeb.arg(ethMaskKey));
            }

            if (serverWeb.hasArg(ethGateKey))
            {
                configNet.ethGate.fromString(serverWeb.arg(ethGateKey));
            }

            if (serverWeb.hasArg(ethDns1Key))
            {
                configNet.ethDns1.fromString(serverWeb.arg(ethDns1Key));
            }

            if (serverWeb.hasArg(ethDns2Key))
            {
                configNet.ethDns2.fromString(serverWeb.arg(ethDns2Key));
            }

            configNet.wifiEnable = serverWeb.hasArg(wifiEnblKey) == true;

            configNet.wifiDhcp = serverWeb.hasArg(wifiDhcpKey) == true;

            if (serverWeb.hasArg(wifiModeKey))
            {
                configNet.wifiMode = serverWeb.arg(wifiModeKey).toInt();
            }
            if (serverWeb.hasArg(wifiPwrKey))
            {
                const uint8_t pwr = serverWeb.arg(wifiPwrKey).toInt();
                configNet.wifiPower = static_cast<wifi_power_t>(pwr);
            }

            if (serverWeb.arg(wifiSsidKey))
            {
                strncpy(configNet.wifiSsid, serverWeb.arg(wifiSsidKey).c_str(), sizeof(configNet.wifiSsid) - 1);
                configNet.wifiSsid[sizeof(configNet.wifiSsid) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.arg(wifiPassKey))
            {
                strncpy(configNet.wifiPass, serverWeb.arg(wifiPassKey).c_str(), sizeof(configNet.wifiPass) - 1);
                configNet.wifiPass[sizeof(configNet.wifiPass) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(wifiIpKey))
            {
                configNet.wifiIp.fromString(serverWeb.arg(wifiIpKey));
            }

            if (serverWeb.hasArg(wifiMaskKey))
            {
                configNet.wifiMask.fromString(serverWeb.arg(wifiMaskKey));
            }

            if (serverWeb.hasArg(wifiGateKey))
            {
                configNet.wifiGate.fromString(serverWeb.arg(wifiGateKey));
            }

            if (serverWeb.hasArg(wifiDns1Key))
            {
                configNet.wifiDns1.fromString(serverWeb.arg(wifiDns1Key));
            }

            if (serverWeb.hasArg(wifiDns2Key))
            {
                configNet.wifiDns2.fromString(serverWeb.arg(wifiDns2Key));
            }

            saveNetworkConfig(configNet);
            serverWeb.send(HTTP_CODE_OK, contTypeText, "ok");
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
                WiFi.begin(configNet.wifiSsid, configNet.wifiPass);
            }
        }
        break;
        case API_PAGE_ZIGBEE:
        {
            if (serverWeb.hasArg(coordMode))
            {
                const uint8_t mode = serverWeb.arg(coordMode).toInt();
                if (mode <= 2 && mode >= 0)
                {
                    configSys.workMode = static_cast<WORK_MODE_t>(mode);
                }
            }

            const char *baud = "baud";
            if (serverWeb.hasArg(baud))
            {
                configSys.serialSpeed = serverWeb.arg(baud).toInt();
            }

            if (serverWeb.hasArg(portKey))
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

            const char *defaultCreds = "admin";
            const char *defaultFwIp = "0.0.0.0";

            if (serverWeb.hasArg(webUserKey))
            {
                if (serverWeb.arg(webUserKey).length() > 0)
                {
                    strncpy(configSys.webUser, serverWeb.arg(webUserKey).c_str(), sizeof(configSys.webUser) - 1);
                }
                else
                {
                    strncpy(configSys.webUser, defaultCreds, sizeof(configSys.webUser) - 1);
                }
                configSys.webUser[sizeof(configSys.webUser) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            if (serverWeb.hasArg(webPassKey))
            {
                if (serverWeb.arg(webPassKey).length() > 0)
                {
                    strncpy(configSys.webPass, serverWeb.arg(webPassKey).c_str(), sizeof(configSys.webPass) - 1);
                }
                else
                {
                    strncpy(configSys.webPass, defaultCreds, sizeof(configSys.webPass) - 1);
                }
                configSys.webPass[sizeof(configSys.webPass) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            configSys.fwEnabled = serverWeb.hasArg(fwEnabledKey) == true;

            if (serverWeb.hasArg(fwIpKey))
            {
                if (serverWeb.arg(fwIpKey).length() > 0)
                {
                    configSys.fwIp.fromString(serverWeb.arg(fwIpKey));
                }
                else
                {
                    configSys.fwIp.fromString(defaultFwIp);
                }
            }

            if (serverWeb.hasArg(fwMaskKey))
            {
                if (serverWeb.arg(fwMaskKey).length() > 0)
                {
                    configSys.fwMask.fromString(serverWeb.arg(fwMaskKey));
                }
                else
                {
                    configSys.fwMask.fromString(defaultFwIp);
                }
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

            const char *mqttReconnectKey = "mqttReconnect";
            if (serverWeb.hasArg(mqttReconnectKey))
            {
                configMqtt.reconnectInt = serverWeb.arg(mqttReconnectKey).toInt();
            }

            const char *MqttDiscoveryKey = "MqttDiscovery";
            configMqtt.discovery = serverWeb.hasArg(MqttDiscoveryKey) == true;

            saveMqttConfig(configMqtt);
        }
        break;
        case API_PAGE_VPN:
        {
            configVpn.wgEnable = serverWeb.hasArg(wgEnableKey) == true;
            if (serverWeb.hasArg(wgLocalIPKey))
            {
                configVpn.wgLocalIP.fromString(serverWeb.arg(wgLocalIPKey));
            }
            if (serverWeb.hasArg(wgLocalSubnetKey))
            {
                configVpn.wgLocalSubnet.fromString(serverWeb.arg(wgLocalSubnetKey));
            }
            if (serverWeb.hasArg(wgLocalPortKey))
            {
                configVpn.wgLocalPort = serverWeb.arg(wgLocalPortKey).toInt();
            }
            if (serverWeb.hasArg(wgLocalGatewayKey))
            {
                configVpn.wgLocalGateway.fromString(serverWeb.arg(wgLocalGatewayKey));
            }
            if (serverWeb.hasArg(wgLocalPrivKeyKey))
            {
                strncpy(configVpn.wgLocalPrivKey, serverWeb.arg(wgLocalPrivKeyKey).c_str(), sizeof(configVpn.wgLocalPrivKey) - 1);
                configVpn.wgLocalPrivKey[sizeof(configVpn.wgLocalPrivKey) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            if (serverWeb.hasArg(wgEndAddrKey))
            {
                strncpy(configVpn.wgEndAddr, serverWeb.arg(wgEndAddrKey).c_str(), sizeof(configVpn.wgEndAddr) - 1);
                configVpn.wgEndAddr[sizeof(configVpn.wgEndAddr) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            if (serverWeb.hasArg(wgEndPubKeyKey))
            {
                strncpy(configVpn.wgEndPubKey, serverWeb.arg(wgEndPubKeyKey).c_str(), sizeof(configVpn.wgEndPubKey) - 1);
                configVpn.wgEndPubKey[sizeof(configVpn.wgEndPubKey) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            if (serverWeb.hasArg(wgEndPortKey))
            {
                configVpn.wgEndPort = serverWeb.arg(wgEndPortKey).toInt();
            }
            if (serverWeb.hasArg(wgAllowedIPKey))
            {
                configVpn.wgAllowedIP.fromString(serverWeb.arg(wgAllowedIPKey));
            }
            if (serverWeb.hasArg(wgAllowedMaskKey))
            {
                configVpn.wgAllowedMask.fromString(serverWeb.arg(wgAllowedMaskKey));
            }
            configVpn.wgMakeDefault = serverWeb.hasArg(wgMakeDefaultKey) == true;
            if (serverWeb.hasArg(wgPreSharedKeyKey))
            {
                strncpy(configVpn.wgPreSharedKey, serverWeb.arg(wgPreSharedKeyKey).c_str(), sizeof(configVpn.wgPreSharedKey) - 1);
                configVpn.wgPreSharedKey[sizeof(configVpn.wgPreSharedKey) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            configVpn.hnEnable = serverWeb.hasArg(hnEnableKey) == true;
            if (serverWeb.hasArg(hnJoinCodeKey))
            {
                strncpy(configVpn.hnJoinCode, serverWeb.arg(hnJoinCodeKey).c_str(), sizeof(configVpn.hnJoinCode) - 1);
                configVpn.hnJoinCode[sizeof(configVpn.hnJoinCode) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            if (serverWeb.hasArg(hnHostNameKey))
            {
                strncpy(configVpn.hnHostName, serverWeb.arg(hnHostNameKey).c_str(), sizeof(configVpn.hnHostName) - 1);
                configVpn.hnHostName[sizeof(configVpn.hnHostName) - 1] = '\0'; // Guarantee a null terminator at the end
            }
            if (serverWeb.hasArg(hnDashUrlKey))
            {
                strncpy(configVpn.hnDashUrl, serverWeb.arg(hnDashUrlKey).c_str(), sizeof(configVpn.hnDashUrl) - 1);
                configVpn.hnDashUrl[sizeof(configVpn.hnDashUrl) - 1] = '\0'; // Guarantee a null terminator at the end
            }

            saveVpnConfig(configVpn);
        }
        break;
        }
        // String cfg = makeJsonConfig(&configNet, &configVpn, &configMqtt, &configSys);
        // LOGD("\n%s",cfg.c_str());
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
    obj[wifiEnblKey] = config.wifiEnable;
    obj[wifiSsidKey] = config.wifiSsid;
    obj[wifiPassKey] = config.wifiPass;
    obj[wifiDhcpKey] = config.wifiDhcp;
    obj[wifiIpKey] = config.wifiIp.toString();
    obj[wifiMaskKey] = config.wifiMask.toString();
    obj[wifiGateKey] = config.wifiGate.toString();
    obj[wifiDns1Key] = config.wifiDns1.toString();
    obj[wifiDns2Key] = config.wifiDns2.toString();
    obj[wifiPwrKey] = config.wifiPower;
    obj[wifiModeKey] = config.wifiMode;
    obj[ethEnblKey] = config.ethEnable;
    obj[ethDhcpKey] = config.ethDhcp;
    obj[ethIpKey] = config.ethIp.toString();
    obj[ethMaskKey] = config.ethMask.toString();
    obj[ethGateKey] = config.ethGate.toString();
    obj[ethDns1Key] = config.ethDns1.toString();
    obj[ethDns2Key] = config.ethDns2.toString();
}

// Serialization VpnConfigStruct into JSON
void serializeVpnConfigToJson(const VpnConfigStruct &config, JsonObject obj)
{
    // WireGuard
    obj[wgEnableKey] = config.wgEnable;
    obj[wgLocalIPKey] = config.wgLocalIP.toString();
    obj[wgLocalSubnetKey] = config.wgLocalSubnet.toString();
    obj[wgLocalPortKey] = config.wgLocalPort;
    obj[wgLocalGatewayKey] = config.wgLocalGateway.toString();
    obj[wgLocalPrivKeyKey] = config.wgLocalPrivKey;
    obj[wgEndAddrKey] = config.wgEndAddr;
    obj[wgEndPubKeyKey] = config.wgEndPubKey;
    obj[wgEndPortKey] = config.wgEndPort;
    obj[wgAllowedIPKey] = config.wgAllowedIP.toString();
    obj[wgAllowedMaskKey] = config.wgAllowedMask.toString();
    obj[wgMakeDefaultKey] = config.wgMakeDefault;
    obj[wgPreSharedKeyKey] = config.wgPreSharedKey;
    // Husarnet
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
    // obj[serverIPKey] = config.serverIP.toString();
    obj[portKey] = config.port;
    obj[userKey] = config.user;
    obj[passKey] = config.pass;
    obj[topicKey] = config.topic;
    obj[updateIntKey] = config.updateInt;
    obj[discoveryKey] = config.discovery;
    obj[reconnectIntKey] = config.reconnectInt;
    // obj[heartbeatTimeKey] = config.heartbeatTime;
}

// Serialization SystemConfigStruct into JSON
void serializeSystemConfigToJson(const SystemConfigStruct &config, JsonObject obj)
{
    // obj[keepWebKey] = config.keepWeb;
    obj[disableWebKey] = config.disableWeb;
    obj[webAuthKey] = config.webAuth;
    obj[webUserKey] = config.webUser;
    obj[webPassKey] = config.webPass;
    obj[fwEnabledKey] = config.fwEnabled;
    obj[fwIpKey] = config.fwIp.toString();
    obj[fwMaskKey] = config.fwMask.toString();
    obj[serialSpeedKey] = config.serialSpeed;
    obj[socketPortKey] = config.socketPort;
    obj[tempOffsetKey] = config.tempOffset;
    obj[disableLedUSBKey] = config.disableLedUSB;
    obj[disableLedPwrKey] = config.disableLedPwr;
    obj[refreshLogsKey] = config.refreshLogs;
    obj[hostnameKey] = config.hostname;
    obj[timeZoneKey] = config.timeZone;
    obj[ntpServ1Key] = config.ntpServ1;
    obj[ntpServ2Key] = config.ntpServ2;
    obj[nmEnableKey] = config.nmEnable;
    obj[nmStartHourKey] = config.nmStart;
    obj[nmEndHourKey] = config.nmEnd;
    // obj[prevWorkModeKey] = static_cast<int>(config.prevWorkMode);
    obj[workModeKey] = static_cast<int>(config.workMode);

    obj[zbRoleKey] = static_cast<int>(config.zbRole);
    obj[zbFwKey] = config.zbFw;

    obj[updCheckTimeKey] = config.updCheckTime;
    obj[updCheckDayKey] = config.updCheckDay;
    obj[updAutoInstKey] = config.updAutoInst;
}

// Serializing system variables to JSON
void serializeSysVarsToJson(const SysVarsStruct &vars, JsonObject obj)
{
    obj[hwBtnIsKey] = vars.hwBtnIs;
    obj[hwLedUsbIsKey] = vars.hwLedUsbIs;
    obj[hwLedPwrIsKey] = vars.hwLedPwrIs;
    // obj[hwUartSelIsKey] = vars.hwUartSelIs;
    obj[hwZigbeeIsKey] = vars.hwZigbeeIs;

    obj[connectedClientsKey] = vars.connectedClients;
    obj[socketTimeKey] = vars.socketTime;
    obj[connectedEtherKey] = vars.connectedEther;
    obj[apStartedKey] = vars.apStarted;
    obj[wifiWebSetupInProgressKey] = vars.wifiWebSetupInProgress;

    obj[vpnWgInitKey] = vars.vpnWgInit;
    obj[vpnWgConnectKey] = vars.vpnWgConnect;
    obj[vpnWgPeerIpKey] = vars.vpnWgPeerIp.toString();
    obj[vpnWgCheckTimeKey] = vars.vpnWgCheckTime;
    obj[vpnHnInitKey] = vars.vpnHnInit;

    obj[mqttConnKey] = vars.mqttConn;
    obj[mqttReconnectTimeKey] = vars.mqttReconnectTime;
    obj[mqttHeartbeatTimeKey] = vars.mqttHeartbeatTime;

    obj[disableLedsKey] = vars.disableLeds;
    // obj[zbLedStateKey] = vars.zbLedState;
    obj[zbFlashingKey] = vars.zbFlashing;

    obj[deviceIdKey] = vars.deviceId;

    obj[espUpdAvailKey] = vars.updateEspAvail;
    obj[rcpUpdAvailKey] = vars.updateZbAvail;
}

void serializeHwConfigToJson(const ThisConfigStruct &config, JsonObject obj)
{
    obj[boardKey] = config.board;
    obj[addrKey] = config.eth.addr;
    obj[pwrPinKey] = config.eth.pwrPin;
    obj[mdcPinKey] = config.eth.mdcPin;
    obj[mdiPinKey] = config.eth.mdiPin;
    obj[phyTypeKey] = config.eth.phyType;
    obj[clkModeKey] = config.eth.clkMode;
    // obj[pwrAltPin] = config.eth.pwrAltPin;
    obj[btnPinKey] = config.mist.btnPin;
    obj[btnPlrKey] = config.mist.btnPlr;
    obj[uartSelPinKey] = config.mist.uartSelPin;
    obj[uartSelPlrKey] = config.mist.uartSelPlr;
    obj[ledModePinKey] = config.mist.ledModePin;
    obj[ledModePlrKey] = config.mist.ledModePlr;
    obj[ledPwrPinKey] = config.mist.ledPwrPin;
    obj[ledPwrPlrKey] = config.mist.ledPwrPlr;
    obj[zbTxPinKey] = config.zb.txPin;
    obj[zbRxPinKey] = config.zb.rxPin;
    obj[zbRstPinKey] = config.zb.rstPin;
    obj[zbBslPinKey] = config.zb.bslPin;
}

bool loadFileConfigHW() // Support for old config HW files
{

    File configFile = LittleFS.open(configFileHw, FILE_READ);

    /*if (!configFile)
    {
        if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
        {
            LOGD("Error with LITTLEFS");
        }
        DynamicJsonDocument config(300);
        config[boardKey] = "";
        writeDefaultConfig(configFileHw, config);
        configFile = LittleFS.open(configFileHw, FILE_READ);
    }*/

    DynamicJsonDocument config(1024);

    if (configFile)
    {

        deserializeJson(config, configFile);

        configFile.close();

        strlcpy(hwConfig.board, config[boardKey] | "", sizeof(hwConfig.board));
        hwConfig.eth.addr = config[addrKey];
        hwConfig.eth.pwrPin = config[pwrPinKey];
        hwConfig.eth.mdcPin = config[mdcPinKey];
        hwConfig.eth.mdiPin = config[mdiPinKey];
        hwConfig.eth.phyType = config[phyTypeKey];
        hwConfig.eth.clkMode = config[clkModeKey];
        if (hwConfig.eth.pwrPin == -1)
        {
            hwConfig.eth.pwrPin = config[pwrAltPinKey];
        }
        // hwConfig.eth.pwrAltPin = config[pwrAltPin];
        hwConfig.mist.btnPin = config[btnPinKey];
        hwConfig.mist.btnPlr = config[btnPlrKey];
        hwConfig.mist.uartSelPin = config[uartSelPinKey];
        hwConfig.mist.uartSelPlr = config[uartSelPlrKey];
        hwConfig.mist.ledModePin = config[ledModePinKey];
        hwConfig.mist.ledModePlr = config[ledModePlrKey];
        hwConfig.mist.ledPwrPin = config[ledPwrPinKey];
        hwConfig.mist.ledPwrPlr = config[ledPwrPlrKey];
        hwConfig.zb.txPin = config[zbTxPinKey];
        hwConfig.zb.rxPin = config[zbRxPinKey];
        hwConfig.zb.rstPin = config[zbRstPinKey];
        hwConfig.zb.bslPin = config[zbBslPinKey];

        LOGD("Removing HW config file");
        LittleFS.remove(configFileHw);
        if (hwConfig.board[0] != '\0' && strlen(hwConfig.board) > 0)
        {
            LOGD("Load HW - OK");
            saveHwConfig(hwConfig);
            return true;
        }
        else
        {
            LOGI("Load HW - ERROR. File is empty");
            return false;
        }
    }
    else
    {
        return false;
    }
}
/*
if (hwConfig.board[0] != '\0' && strlen(hwConfig.board) > 0)
{
    delay(3000);
    LOGD("Load HW - OK");
    saveHwConfig(hwConfig);
    return true;
}
else
{
    LOGI("Load HW - ERROR");

    int searchId = 0;
    if (config["searchId"])
    {
        searchId = config["searchId"];
    }


    String chipId = ESP.getChipModel();
    LOGW("%s", chipId.c_str());
     if (chipId == "ESP32-D0WDQ6")
    {
         searchId = 12;
     }


    ThisConfigStruct *newConfig = findBrdConfig(searchId);
    if (newConfig)
    {
        LOGD("Find. Saving config");
        saveHwConfig(*newConfig);


        DynamicJsonDocument config(512);
        config[boardKey] = newConfig->board;
        config[addrKey] = newConfig->eth.addr;
        config[pwrPinKey] = newConfig->eth.pwrPin;
        config[mdcPinKey] = newConfig->eth.mdcPin;
        config[mdiPinKey] = newConfig->eth.mdiPin;
        config[phyTypeKey] = newConfig->eth.phyType;
        config[clkModeKey] = newConfig->eth.clkMode;
        // config[pwrAltPin] = newConfig->eth.pwrAltPin;
        config[btnPinKey] = newConfig->mist.btnPin;
        config[btnPlrKey] = newConfig->mist.btnPlr;
        config[uartSelPinKey] = newConfig->mist.uartSelPin;
        config[uartSelPlrKey] = newConfig->mist.uartSelPlr;
        config[ledModePinKey] = newConfig->mist.ledModePin;
        config[ledModePlrKey] = newConfig->mist.ledModePlr;
        config[ledPwrPinKey] = newConfig->mist.ledPwrPin;
        config[ledPwrPlrKey] = newConfig->mist.ledPwrPlr;
        config[zbTxPinKey] = newConfig->zb.txPin;
        config[zbRxPinKey] = newConfig->zb.rxPin;
        config[zbRstPinKey] = newConfig->zb.rstPin;
        config[zbBslPinKey] = newConfig->zb.bslPin;
        writeDefaultConfig(configFileHw, config);


        LOGD("Calc and save temp offset");
        float CPUtemp = getCPUtemp(true);
        int offset = CPUtemp - 30;
        systemCfg.tempOffset = int(offset);
        saveSystemConfig(systemCfg);

        restartDevice();
    }
}
return false;
*/

/* Previous firmware read config support. start */

/*
const char *msg_file_rm = "OK. Remove old file";
const char *msg_open_f = "Error. open failed";

void fileReadError(DeserializationError error, const char *fileName)
{
    String fileContent = "";
    File configFile = LittleFS.open(fileName, FILE_READ);
    if (!configFile)
    {
        return;
    }
    while (configFile.available())
    {
        fileContent += (char)configFile.read();
    }
    LOGI("%s - %s - %s", fileName, error.c_str(), fileContent.c_str());
    configFile.close();
    if (error == DeserializationError::EmptyInput)
    {
        LOGD("%s %s", fileName, msg_file_rm);
        LittleFS.remove(fileName);
    }
}


bool loadFileSystemVar()
{
    File configFile = LittleFS.open(configFileSystem, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileSystem, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileSystem);
        return false;
    }

    systemCfg.tempOffset = (int)doc[tempOffsetKey];

    configFile.close();
    LOGD("%s %s", configFileSystem, msg_file_rm);
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileSystem);
    return true;
}

bool loadFileConfigWifi()
{
    const char *enableWiFi = "enableWiFi";
    const char *ssid = "ssid";
    const char *dhcpWiFi = "dhcpWiFi";
    const char *ip = "ip";
    const char *mask = "mask";
    const char *gw = "gw";

    File configFile = LittleFS.open(configFileWifi, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileWifi, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileWifi);
        return false;
    }

    networkCfg.wifiDhcp = (int)doc[dhcpWiFi];
    strlcpy(networkCfg.wifiSsid, doc[ssid] | "", sizeof(networkCfg.wifiSsid));
    strlcpy(networkCfg.wifiPass, doc[passKey] | "", sizeof(networkCfg.wifiPass));

    networkCfg.wifiIp.fromString(doc[ip] | "");
    networkCfg.wifiMask.fromString(doc[mask] | "");
    networkCfg.wifiGate.fromString(doc[gw] | "");

    configFile.close();
    LOGD("%s %s", configFileWifi, msg_file_rm);
    saveNetworkConfig(networkCfg);
    LittleFS.remove(configFileWifi);
    return true;
}

bool loadFileConfigEther()
{
    const char *dhcp = "dhcp";
    const char *ip = "ip";
    const char *mask = "mask";
    const char *gw = "gw";

    File configFile = LittleFS.open(configFileEther, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileEther, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileEther);
        return false;
    }

    networkCfg.ethDhcp = (int)doc[dhcp];
    networkCfg.ethIp.fromString(doc[ip] | "");
    networkCfg.ethMask.fromString(doc[mask] | "");
    networkCfg.ethGate.fromString(doc[gw] | "");

    configFile.close();
    LOGD("%s %s", configFileEther, msg_file_rm);
    saveNetworkConfig(networkCfg);
    LittleFS.remove(configFileEther);
    return true;
}

bool loadFileConfigGeneral()
{
    File configFile = LittleFS.open(configFileGeneral, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileGeneral, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileGeneral);
        return false;
    }

    if ((double)doc[refreshLogsKey] < 1)
    {
        systemCfg.refreshLogs = 1;
    }
    else
    {
        systemCfg.refreshLogs = (int)doc[refreshLogsKey] / 1000;
    }

    strlcpy(systemCfg.hostname, doc[hostnameKey] | "", sizeof(systemCfg.hostname));

    systemCfg.workMode = static_cast<WORK_MODE_t>((uint8_t)doc[coordMode]);

    systemCfg.disableLedPwr = (uint8_t)doc[disableLedPwrKey];
    systemCfg.disableLedUSB = (uint8_t)doc[disableLedUSBKey];
    vars.disableLeds = (uint8_t)doc[disableLedsKey];
    // systemCfg.keepWeb = (uint8_t)doc[keepWebKey];
    strlcpy(systemCfg.timeZone, doc[timeZoneKey] | NTP_TIME_ZONE, sizeof(systemCfg.timeZone));

    configFile.close();
    LOGD("%s %s", configFileGeneral, msg_file_rm);
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileGeneral);
    return true;
}

bool loadFileConfigSecurity()
{
    File configFile = LittleFS.open(configFileSecurity, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileSecurity, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileSecurity);
        return false;
    }

    systemCfg.disableWeb = (uint8_t)doc[disableWebKey];
    systemCfg.webAuth = (uint8_t)doc[webAuthKey];
    strlcpy(systemCfg.webUser, doc[webUserKey] | "", sizeof(systemCfg.webUser));
    strlcpy(systemCfg.webPass, doc[webPassKey] | "", sizeof(systemCfg.webPass));
    systemCfg.fwEnabled = (uint8_t)doc[fwEnabledKey];
    systemCfg.fwIp.fromString(doc[fwIpKey] | "0.0.0.0");

    configFile.close();
    LOGD("%s %s", configFileSecurity, msg_file_rm);
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileSecurity);
    return true;
}


bool loadFileConfigSerial()
{
    const char *baud = "baud";

    File configFile = LittleFS.open(configFileSerial, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileSerial, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileSerial);
        return false;
    }

    systemCfg.serialSpeed = (int)doc[baud];
    systemCfg.socketPort = (int)doc[portKey];

    configFile.close();
    LOGD("%s %s", configFileSerial, msg_file_rm);
    saveSystemConfig(systemCfg);
    LittleFS.remove(configFileSerial);
    return true;
}

bool loadFileConfigMqtt()
{
    File configFile = LittleFS.open(configFileMqtt, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileMqtt, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileMqtt);
        return false;
    }

    mqttCfg.enable = (int)doc[enableKey];
    strlcpy(mqttCfg.server, doc[serverKey] | "", sizeof(mqttCfg.server));
    mqttCfg.port = (int)doc[portKey];
    strlcpy(mqttCfg.user, doc[userKey] | "", sizeof(mqttCfg.user));
    strlcpy(mqttCfg.pass, doc[passKey] | "", sizeof(mqttCfg.pass));
    strlcpy(mqttCfg.topic, doc[topicKey] | "", sizeof(mqttCfg.topic));
    mqttCfg.updateInt = (int)doc[intervalKey];
    mqttCfg.discovery = (int)doc[discoveryKey];
    mqttCfg.reconnectInt = (int)doc[reconnectIntKey];

    configFile.close();
    LOGD("%s %s", configFileMqtt, msg_file_rm);
    saveMqttConfig(mqttCfg);
    LittleFS.remove(configFileMqtt);
    return true;
}

bool loadFileConfigWg()
{
    const char *localAddr = "localAddr";
    const char *localPrivKey = "localIP";
    const char *endAddr = "endAddr";
    const char *endPubKey = "endPubKey";
    const char *endPort = "endPort";

    File configFile = LittleFS.open(configFileWg, FILE_READ);
    if (!configFile)
    {
        // LOGD("%s %s", configFileWg, msg_open_f);
        return false;
    }

    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, configFile);

    if (error)
    {
        configFile.close();
        fileReadError(error, configFileWg);
        return false;
    }

    vpnCfg.wgEnable = (int)doc[enableKey];

    vpnCfg.wgLocalIP.fromString(doc[localAddr] | "");

    strlcpy(vpnCfg.wgLocalPrivKey, doc[localPrivKey] | "", sizeof(vpnCfg.wgLocalPrivKey));
    strlcpy(vpnCfg.wgEndAddr, doc[endAddr] | "", sizeof(vpnCfg.wgEndAddr));
    strlcpy(vpnCfg.wgEndPubKey, doc[endPubKey] | "", sizeof(vpnCfg.wgEndPubKey));
    vpnCfg.wgEndPort = (int)doc[endPort];

    configFile.close();
    LOGD("%s %s", configFileWg, msg_file_rm);
    saveVpnConfig(vpnCfg);
    LittleFS.remove(configFileWg);
    return true;
}

*/

/* Previous firmware read config support. end */
