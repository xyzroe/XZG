#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <CircularBuffer.h>
#include "version.h"

// #define DEBUG
// ESP32 PINS TO CONTROL LAN8720
#define ETH_CLK_MODE_1 ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN_ALTERNATIVE_1 5
#define ETH_POWER_PIN_1 -1
#define ETH_TYPE_1 ETH_PHY_LAN8720
#define ETH_ADDR_1 0
#define ETH_MDC_PIN_1 23
#define ETH_MDIO_PIN_1 18
// ESP32 PINS TO CONTROL CC2652P
#define CC2652P_RST 16
#define CC2652P_FLSH 32
#define CC2652P_RXD 36
#define CC2652P_TXD 4
#define BTN 35
#define MODE_SWITCH 33
#define DEBOUNCE_TIME 70
#define PRODUCTION 1
#define TCP_LISTEN_PORT 9999
#define FORMAT_LITTLEFS_IF_FAILED true


// CC2652 settings
#define BSL_PIN 15 // CC2652 pin number
#define BSL_LEVEL 0 //0-LOW 1-HIGH


const int16_t overseerInterval = 5 * 1000; // check lan or wifi connection every 5sec
const uint8_t overseerMaxRetry = 3;         // 5x12 = 60sec delay for AP start
const uint8_t LED_USB = 12;                 // RED
const uint8_t LED_PWR = 14;                 // BLUE
const uint8_t MAX_SOCKET_CLIENTS = 5;

enum COORDINATOR_MODE_t : uint8_t
{
  COORDINATOR_MODE_LAN,
  COORDINATOR_MODE_WIFI,
  COORDINATOR_MODE_USB
};

// struct JsonConsts_t{
//   char* str;
// };
// JsonConsts_t JsonConsts {
//   "sadasd"
// };

struct ConfigSettingsStruct
{
  char ssid[50];
  char password[50];
  char ipAddressWiFi[18];
  char ipMaskWiFi[16];
  char ipGWWiFi[18];
  bool dhcpWiFi;
  bool dhcp;
  bool connectedEther;
  char ipAddress[18];
  char ipMask[16];
  char ipGW[18];
  int serialSpeed;
  int socketPort;
  bool disableWeb;
  int refreshLogs;
  char hostname[50];
  bool connectedSocket[10];
  int connectedClients;
  unsigned long socketTime;
  int tempOffset;
  bool webAuth;
  char webUser[50];
  char webPass[50];
  bool disableLedUSB;
  bool disableLedPwr;
  // bool disablePingCtrl;
  bool disableLeds;
  COORDINATOR_MODE_t coordinator_mode;
  COORDINATOR_MODE_t prevCoordinator_mode; // for button
  bool keepWeb;
  bool apStarted;
  bool wifiWebSetupInProgress;
  bool fwEnabled;
  IPAddress fwIp;
  bool mqttEnable;
  char mqttServer[50];
  IPAddress mqttServerIP;
  int mqttPort;
  char mqttUser[50];
  char mqttPass[50];
  char mqttTopic[50];
  // bool mqttRetain;
  int mqttInterval;
  bool mqttDiscovery;
  unsigned long mqttReconnectTime;
  unsigned long mqttHeartbeatTime;
  bool zbLedState;
  bool zbFlashing;
};

struct InfosStruct
{
  char device[8];
  char mac[8];
  char flash[8];
};

struct zbVerStruct
{
  uint32_t zbRev;
  uint8_t maintrel;
  uint8_t minorrel;
  uint8_t majorrel;
  uint8_t product;
  uint8_t transportrev;
};

typedef CircularBuffer<char, 8024> LogConsoleType;

// #define WL_MAC_ADDR_LENGTH 6

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(String(x))
#define DEBUG_PRINTLN(x) Serial.println(String(x))
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
#endif
