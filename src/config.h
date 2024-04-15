#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <CircularBuffer.hpp>
#include "version.h"
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

#define DEBOUNCE_TIME 70
#define MAX_DEV_ID_LONG 50

#define ZB_TCP_PORT 6638 // any port ever. later setup from config file
#define ZB_SERIAL_SPEED 115200
#define NTP_TIME_ZONE "Europe/Kiev"
#define NTP_SERV_1 "pool.ntp.org"
#define NTP_SERV_2 "time.google.com"
#define DNS_SERV_1 "1.1.1.1"
#define DNS_SERV_2 "8.8.8.8"
#define NETWORK_MASK "255.255.255.0"
#define NM_START_TIME "23:00"
#define NM_END_TIME "07:00"

#define MAX_SOCKET_CLIENTS 5

#define FORMAT_LITTLEFS_IF_FAILED true

#define BUFFER_SIZE 256

#define UPD_FILE "https://github.com/mercenaruss/uzg-firmware/releases/latest/download/XZG.bin"

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

enum LED_t : uint8_t
{
  POWER_LED,
  MODE_LED,
  ZB_LED
};

enum MODE_t : uint8_t
{
  OFF,
  ON,
  TOGGLE,
  BLINK
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

  //WORK_MODE_t workMode; // for button  // WORK_MODE_t

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

  bool disableLeds;
  bool zbLedState;
  bool zbFlashing;

  char deviceId[MAX_DEV_ID_LONG];
};

// Network configuration structure
struct NetworkConfigStruct
{
  // Wi-Fi
  bool wifiEnable;
  char wifiSsid[50];
  char wifiPass[50];
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
  // IPAddress serverIP;
  int port;
  char user[50];
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

  int refreshLogs;
  char hostname[50];

  char timeZone[50];
  char ntpServ1[50];
  char ntpServ2[50];

  bool nmEnable;
  char nmStart[6];
  char nmEnd[6];

  //WORK_MODE_t prevWorkMode; // for button // WORK_MODE_t
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
void getNvsStats(int *total, int *used);
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

struct LEDControl {
    LEDSettings modeLED;
    LEDSettings powerLED;
};
