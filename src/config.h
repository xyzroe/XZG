#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <CircularBuffer.hpp>
#include "version.h"
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include "const/hw.h"

#define DEBOUNCE_TIME 70
#define MAX_DEV_ID_LONG 50

#define ZB_TCP_PORT 6638 // any port ever. later setup from config file
#define ZB_SERIAL_SPEED 115200
#define NTP_TIME_ZONE "Europe/Berlin"
#define NTP_SERV_1 "pool.ntp.org"
#define NTP_SERV_2 "time.google.com"
#define DNS_SERV_1 "1.1.1.1"
#define DNS_SERV_2 "8.8.8.8"
#define NETWORK_MASK "255.255.255.0"
#define NETWORK_ZERO "0.0.0.0"
#define NM_START_TIME "23:00"
#define NM_END_TIME "07:00"
#define UPD_CHK_TIME "01:00"
#define UPD_CHK_DAY "*"

#define MAX_SOCKET_CLIENTS 5

#define FORMAT_LITTLEFS_IF_FAILED true

#define BUFFER_SIZE 256

// CC2652 settings (FOR BSL VALIDATION!)
#define NEED_BSL_PIN 15  // CC2652 pin number (FOR BSL VALIDATION!)
#define NEED_BSL_LEVEL 1 // 0-ERROR 1-LOW 2-HIGH

const int16_t overseerInterval = 5 * 1000; // check lan or wifi connection every 5sec
const uint8_t overseerMaxRetry = 3;        // 5x12 = 60sec delay for AP start

enum WORK_MODE_t : uint8_t
{
  WORK_MODE_NETWORK,
  WORK_MODE_USB
};

enum LED_t : uint8_t
{
  POWER_LED,
  MODE_LED,
  ZB_LED
};

enum ZB_ROLE_t : uint8_t
{
  UNDEFINED,
  COORDINATOR,
  ROUTER,
  OPENTHREAD
};

extern const char *coordMode; // coordMode node name
extern const char *configFileSystem;
extern const char *configFileWifi;
extern const char *configFileEther;
extern const char *configFileGeneral;
extern const char *configFileSecurity;
extern const char *configFileSerial;
extern const char *configFileMqtt;
extern const char *configFileWg;
extern const char *configFileHw;

struct SysVarsStruct
{
  bool hwBtnIs = false;
  bool hwLedUsbIs = false;
  bool hwLedPwrIs = false;
  // bool hwUartSelIs = false;
  bool hwZigbeeIs = false;
  bool oneWireIs = false;

  bool connectedSocket[MAX_SOCKET_CLIENTS]; //[10]
  int connectedClients;
  unsigned long socketTime;

  bool connectedEther = false;
  bool ethIPv6 = false;

  bool apStarted = false;
  bool wifiWebSetupInProgress = false;

  bool vpnWgInit = false;
  bool vpnWgConnect = false;
  IPAddress vpnWgPeerIp;
  unsigned long vpnWgCheckTime;

  bool vpnHnInit = false;
  bool mqttConn = false;
  unsigned long mqttReconnectTime;
  unsigned long mqttHeartbeatTime;

  bool disableLeds;
  // bool zbLedState;
  bool zbFlashing;

  char deviceId[MAX_DEV_ID_LONG];

  bool updateEspAvail;
  bool updateZbAvail;

  char lastESPVer[20];
  char lastZBVer[20];
  //IPAddress savedWifiDNS;
  //IPAddress savedEthDNS;

  bool firstUpdCheck = false;
  
  uint32_t last1wAsk = 0;
  float temp1w = 0;
};

// Network configuration structure
struct NetworkConfigStruct
{
  // Wi-Fi
  bool wifiEnable;
  // int wifiPower;
  wifi_power_t wifiPower;
  int wifiMode;
  char wifiSsid[50];
  char wifiPass[80];
  bool wifiDhcp;
  IPAddress wifiIp;
  IPAddress wifiMask;
  IPAddress wifiGate;
  IPAddress wifiDns1;
  IPAddress wifiDns2;
  // LAN
  bool ethEnable;
  bool ethDhcp;
  IPAddress ethIp;
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
  IPAddress wgLocalSubnet;
  uint16_t wgLocalPort;
  IPAddress wgLocalGateway;
  char wgLocalPrivKey[50];
  char wgEndAddr[50];
  char wgEndPubKey[50];
  uint16_t wgEndPort;
  IPAddress wgAllowedIP;
  IPAddress wgAllowedMask;
  bool wgMakeDefault;
  char wgPreSharedKey[50];

  // Husarnet
  bool hnEnable;
  char hnJoinCode[80];
  char hnHostName[30];
  char hnDashUrl[60];
};

// Function prototypes for VpnConfigStruct
void saveVpnConfig(const VpnConfigStruct &config);
void loadVpnConfig(VpnConfigStruct &config);

// MQTT configuration structure
struct MqttConfigStruct
{
  bool enable;
  char server[60];
  int port;
  char user[30];
  char pass[50];
  char topic[50];
  // bool retain; // commented as per the structure definition
  int updateInt;
  bool discovery;
  int reconnectInt;
  // unsigned long heartbeatTime;
};

// Function prototypes for MqttConfigStruct
void saveMqttConfig(const MqttConfigStruct &config);
void loadMqttConfig(MqttConfigStruct &config);

struct SystemConfigStruct
{
  // bool keepWeb; // when usb mode active

  bool disableWeb; // when socket connected
  bool webAuth;
  char webUser[50];
  char webPass[50];

  bool fwEnabled; // firewall for socket connection
  IPAddress fwIp; // allowed IP base
  IPAddress fwMask; // allowed mask

  int serialSpeed;
  int socketPort;

  int tempOffset;

  bool disableLedUSB;
  bool disableLedPwr;

  int refreshLogs;
  char hostname[50];

  char timeZone[50];
  char ntpServ1[50];
  char ntpServ2[50];

  bool nmEnable;
  char nmStart[6];
  char nmEnd[6];

  // WORK_MODE_t prevWorkMode; // for button // WORK_MODE_t
  WORK_MODE_t workMode;

  ZB_ROLE_t zbRole;
  char zbFw[30];

  char updCheckTime[6];
  char updCheckDay[3];
  bool updAutoInst;
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
void serializeHwConfigToJson(const ThisConfigStruct &config, JsonObject obj);

void updateConfiguration(WebServer &server, SystemConfigStruct &configSys, NetworkConfigStruct &configNet, VpnConfigStruct &configVpn, MqttConfigStruct &configMqtt);

typedef CircularBuffer<char, 8024> LogConsoleType;

void initNVS();
void getNvsStats(int *total, int *used);
void printNVSFreeSpace();
void eraseNVS();

String makeJsonConfig(const NetworkConfigStruct *networkCfg = nullptr,
                      const VpnConfigStruct *vpnCfg = nullptr,
                      const MqttConfigStruct *mqttCfg = nullptr,
                      const SystemConfigStruct *systemCfg = nullptr,
                      const SysVarsStruct *systemVars = nullptr,
                      const ThisConfigStruct *hwCfg = nullptr);

bool loadFileConfigHW();

void saveHwConfig(const ThisConfigStruct &config);
void loadHwConfig(ThisConfigStruct &config);

/* Previous firmware read config support. start */
/*
bool loadFileSystemVar();
bool loadFileConfigWifi();
bool loadFileConfigEther();
bool loadFileConfigGeneral();
bool loadFileConfigSecurity();
bool loadFileConfigSerial();
bool loadFileConfigMqtt();
bool loadFileConfigWg();
*/
/* Previous firmware read config support. end */

/* ----- Define functions | START -----*/
#ifdef __cplusplus
extern "C"
{
#endif
  uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

#define STRINGIFY(s) STRINGIFY1(s) // Don’t ask why. It has to do with the inner workings of the preprocessor.
#define STRINGIFY1(s) #s           // https://community.platformio.org/t/how-to-put-a-string-in-a-define-in-build-flag-into-a-libary-json-file/13480/6

// #define min(a, b) ((a) < (b) ? (a) : (b))

// Define log levels
#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

#ifdef DEBUG

// Set the current logging level here
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG

//#define DEBUG_PRINT(x) Serial.print(String(x))
//#define DEBUG_PRINTLN(x) Serial.println(String(x))

#else

// Set the current logging level here
#define CURRENT_LOG_LEVEL LOG_LEVEL_INFO

//#define DEBUG_PRINT(x)
//#define DEBUG_PRINTLN(x)
#endif

#endif

// Define ANSI color codes
#define ANSI_COLOR_PURPLE "\x1b[35m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Conditional logging macros
#if CURRENT_LOG_LEVEL >= LOG_LEVEL_WARN
#define LOGW(format, ...)                                                                                                                           \
  if (systemCfg.workMode == WORK_MODE_NETWORK)                                                                                                      \
  {                                                                                                                                                 \
    Serial.printf(ANSI_COLOR_PURPLE "%lu " ANSI_COLOR_RESET ANSI_COLOR_RED "[%s] " ANSI_COLOR_RESET format "\n", millis(), __func__, ##__VA_ARGS__); \
  }
#else
#define LOGW(format, ...) // Nothing
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOGI(format, ...)                                                                                                                             \
  if (systemCfg.workMode == WORK_MODE_NETWORK)                                                                                                        \
  {                                                                                                                                                   \
    Serial.printf(ANSI_COLOR_PURPLE "%lu " ANSI_COLOR_RESET ANSI_COLOR_GREEN "[%s] " ANSI_COLOR_RESET format "\n", millis(), __func__, ##__VA_ARGS__); \
  }
#else
#define LOGI(format, ...) // Nothing
#endif

#if CURRENT_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOGD(format, ...)                                                                                                                              \
  if (systemCfg.workMode == WORK_MODE_NETWORK)                                                                                                         \
  {                                                                                                                                                    \
    Serial.printf(ANSI_COLOR_PURPLE "%lu " ANSI_COLOR_RESET ANSI_COLOR_YELLOW "[%s] " ANSI_COLOR_RESET format "\n", millis(), __func__, ##__VA_ARGS__); \
  }
#else
#define LOGD(format, ...) // Nothing
#endif

/* ----- Define functions | END -----*/

enum LEDMode
{
  LED_OFF,
  LED_ON,
  LED_TOGGLE,
  LED_BLINK_5T,
  LED_BLINK_1T,
  LED_BLINK_1Hz,
  LED_BLINK_3Hz,
  LED_FLASH_1Hz,
  LED_FLASH_3Hz
};

struct LEDSettings
{
  String name = "";
  int pin = -1;
  LEDMode mode;
  bool active;
};

struct LEDControl
{
  LEDSettings modeLED;
  LEDSettings powerLED;
};

enum usbMode : uint8_t
{
  XZG,
  ZIGBEE
};

enum updInfoType : uint8_t
{
    UPD_ESP,
    UPD_ZB
};