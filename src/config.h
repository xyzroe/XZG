#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <CircularBuffer.hpp>
#include "version.h"
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

#define DEBOUNCE_TIME 70

#define ZB_TCP_PORT 9999 // any port ever. later setup from config file
#define ZB_SERIAL_SPEED 115200
#define NTP_TIME_ZONE "Europe/Kiev"
#define NTP_SERV_1 "pool.ntp.org"
#define NTP_SERV_2 "time.google.com"
#define DNS_SERV_1 "1.1.1.1"
#define DNS_SERV_2 "8.8.8.8"
#define NETWORK_MASK "255.255.255.0"

#define MAX_SOCKET_CLIENTS 5

#define FORMAT_LITTLEFS_IF_FAILED true

// CC2652 settings (FOR BSL VALIDATION!)
#define NEED_BSL_PIN 15  // CC2652 pin number (FOR BSL VALIDATION!)
#define NEED_BSL_LEVEL 0 // 0-LOW 1-HIGH

const int16_t overseerInterval = 5 * 1000; // check lan or wifi connection every 5sec
const uint8_t overseerMaxRetry = 3;        // 5x12 = 60sec delay for AP start

enum WORK_MODE_t : uint8_t
{
  WORK_MODE_NETWORK,
  // COORDINATOR_MODE_WIFI,
  WORK_MODE_USB
};

extern const char *coordMode;     // coordMode node name
extern const char *prevCoordMode; // prevCoordMode node name
extern const char *configFileSystem;
extern const char *configFileWifi;
extern const char *configFileEther;
extern const char *configFileGeneral;
extern const char *configFileSecurity;
extern const char *configFileSerial;
extern const char *configFileMqtt;
extern const char *configFileWg;
extern const char *configFileHw;
// extern const char *deviceModel;


struct SysVarsStruct
{
    bool hwBtnIs = false;
    bool hwLedUsbIs = false;
    bool hwLedPwrIs = false;
    bool hwUartSelIs = false;
    bool hwZigbeeIs = false;

    WORK_MODE_t workMode; // for button  // WORK_MODE_t

    bool connectedSocket[MAX_SOCKET_CLIENTS]; //[10]
    int connectedClients;
    unsigned long socketTime;

    bool connectedEther = false;

    bool apStarted = false;
    bool wifiWebSetupInProgress = false;

    bool vpnWgInit = false;
    bool vpnWgConnect = false;
    IPAddress vpnWgPeerIp;
    unsigned long vpnWgCheckTime;

    bool vpnHnInit = false;
    bool mqttConn = true;
    unsigned long mqttReconnectTime;
    unsigned long mqttHeartbeatTime;

    bool zbLedState;
    bool zbFlashing;
};

// Network configuration structure
struct NetworkConfigStruct
{
  // Wi-Fi
  bool wifiEnable;
  char wifiSsid[50];
  char wifiPassword[50];
  bool wifiDhcp;
  IPAddress wifiAddr;
  IPAddress wifiMask;
  IPAddress wifiGate;
  IPAddress wifiDns1;
  IPAddress wifiDns2;
  // LAN
  bool ethEnable;
  bool ethDhcp;
  IPAddress ethAddr;
  IPAddress ethMask;
  IPAddress ethGate;
  IPAddress ethDns1;
  IPAddress ethDns2;
};

// Function prototypes for NetworkConfigStruct
void saveNetworkConfig(const NetworkConfigStruct &config);
void loadNetworkConfig(NetworkConfigStruct &config);

// VPN configuration structure
struct VpnConfigStruct
{
  // Wireguard
  bool wgEnable;
  IPAddress wgLocalIP;
  char wgLocalPrivKey[50];
  char wgEndAddr[50];
  char wgEndPubKey[50];
  int wgEndPort;
  // Husarnet
  bool hnEnable;
  char hnJoinCode[80];
  char hnHostName[30];
  char hnDashUrl[30];
};

// Function prototypes for VpnConfigStruct
void saveVpnConfig(const VpnConfigStruct &config);
void loadVpnConfig(VpnConfigStruct &config);

// MQTT configuration structure
struct MqttConfigStruct
{
  bool enable;
  // bool connect;
  char server[50];
  //IPAddress serverIP;
  int port;
  char user[50];
  char pass[50];
  char topic[50];
  // bool retain; // commented as per the structure definition
  int updateInt;
  bool discovery;
  int reconnectInt;
  //unsigned long heartbeatTime;
};

// Function prototypes for MqttConfigStruct
void saveMqttConfig(const MqttConfigStruct &config);
void loadMqttConfig(MqttConfigStruct &config);

struct SystemConfigStruct
{
  bool keepWeb; // when usb mode active

  bool disableWeb; // when socket connected
  bool webAuth;
  char webUser[50];
  char webPass[50];

  bool fwEnabled; // firewall for socket connection
  IPAddress fwIp; // allowed IP

  int serialSpeed;
  int socketPort;

  int tempOffset;

  bool disableLedUSB;
  bool disableLedPwr;
  bool disableLeds;

  int refreshLogs;
  char hostname[50];

  char timeZone[50];
  char ntpServ1[50];
  char ntpServ2[50];

  WORK_MODE_t prevWorkMode; // for button // WORK_MODE_t
  WORK_MODE_t workMode;     // for button  // WORK_MODE_t
};

// Function prototypes for SystemConfigStruct
void saveSystemConfig(const SystemConfigStruct &config);
void loadSystemConfig(SystemConfigStruct &config);

// Serialization function declarations
void serializeNetworkConfigToJson(const NetworkConfigStruct &config, JsonObject obj);
void serializeVpnConfigToJson(const VpnConfigStruct &config, JsonObject obj);
void serializeMqttConfigToJson(const MqttConfigStruct &config, JsonObject obj);
void serializeSystemConfigToJson(const SystemConfigStruct &config, JsonObject obj);
void serializeSysVarsToJson(const SysVarsStruct &vars, JsonObject obj);

void updateConfiguration(WebServer &server, SystemConfigStruct &configSys, NetworkConfigStruct &configNet, VpnConfigStruct &configVpn, MqttConfigStruct &configMqtt);

/*
struct ConfigSettingsStruct
{
  char ssid[50]; //+
  char password[50]; //+
  char ipAddressWiFi[18]; //+
  char ipMaskWiFi[16]; //+
  char ipGWWiFi[18]; //+
  bool dhcpWiFi; //+
  bool dhcp; //+
  *bool connectedEther; //+
  char ipAddress[18]; //+
  char ipMask[16]; //+
  char ipGW[18]; //+
  int serialSpeed; //+
  int socketPort; //+
  bool disableWeb; //+
  int refreshLogs; //+
  char hostname[50]; //+
  bool connectedSocket[10]; //+
  int connectedClients; //+
  unsigned long socketTime; //+
  int tempOffset; //+
  bool webAuth; //+
  char webUser[50]; //+
  char webPass[50]; //+
  bool disableLedUSB; //+
  bool disableLedPwr; //+
  // bool disablePingCtrl;
  bool disableLeds; //+
  WORK_MODE_t coordinator_mode;  //+
  WORK_MODE_t prevCoordinator_mode; // for button  //+
  bool keepWeb; //+
  bool apStarted;  //+
  bool wifiWebSetupInProgress;
  bool fwEnabled; //+
  IPAddress fwIp; //+

  bool zbLedState; //+
  bool zbFlashing; //+
  char timeZone[50]; //+
};*/
/*
struct MqttSettingsStruct
{
  bool enable;
  bool connect = 0;
  char server[50];
  IPAddress serverIP;
  int port;
  char user[50];
  char pass[50];
  char topic[50];
  // bool retain;
  int interval;
  bool discovery;
  unsigned long reconnectTime;
  unsigned long heartbeatTime;
};*/
/*
struct WgSettingsStruct
{
  bool enable;
  bool init = 0;
  char localAddr[16];
  IPAddress localIP;
  char localPrivKey[45];
  char endAddr[45];
  char endPubKey[45];
  int endPort;
};*/

struct zbVerStruct
{
  uint32_t zbRev;
  uint8_t maintrel;
  uint8_t minorrel;
  uint8_t majorrel;
  uint8_t product;
  uint8_t transportrev;
  String chipID;
};

typedef CircularBuffer<char, 8024> LogConsoleType;

// #define WL_MAC_ADDR_LENGTH 6

void initNVS();
void printNVSFreeSpace();
void eraseNVS();
String makeJsonConfig(const NetworkConfigStruct *networkCfg = nullptr,
                      const VpnConfigStruct *vpnCfg = nullptr,
                      const MqttConfigStruct *mqttCfg = nullptr,
                      const SystemConfigStruct *systemCfg = nullptr,
                      const SysVarsStruct *systemVars = nullptr);

bool loadFileConfigHW();

/* Previous firmware read config support. start */
bool loadFileSystemVar();
bool loadFileConfigWifi();
bool loadFileConfigEther();
bool loadFileConfigGeneral();
bool loadFileConfigSecurity();
bool loadFileConfigSerial();
bool loadFileConfigMqtt();
bool loadFileConfigWg();
// IPAddress parse_ip_address(const char *str);
/* Previous firmware read config support. end */

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(String(x))
#define DEBUG_PRINTLN(x) Serial.println(String(x))

#define LOGE(tag, format, ...) Serial.printf("[%s] " format " (%s:%d)\n", tag, ##__VA_ARGS__, __FILE__, __LINE__)
#define LOGI(tag, format, ...) Serial.printf("[%s] " format "\n", tag, ##__VA_ARGS__)

/*
      LOGE("ZB", "%u - %s", 1, "test");
      LOGI("ZB", "%u - %s", 1, "test");


      [ZB] 1 - test (src/main.cpp:1374)
      [ZB] 1 - test
*/

#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define LOGE(f_, ...)
#define LOGI(f_, ...)
#endif
#endif
