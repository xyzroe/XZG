#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <CircularBuffer.h>

#include "version.h"

//---WT32-ETH01---
//LAN
#define ETH_CLK_MODE_1 ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN_1 16
#define ETH_TYPE_1 ETH_PHY_LAN8720
#define ETH_ADDR_1 1
#define ETH_MDC_PIN_1 23
#define ETH_MDIO_PIN_1 18
//ZIGBEE
#define RESTART_ZIGBEE_1 33
#define FLASH_ZIGBEE_1 32
#define ZRXD_1 5
#define ZTXD_1 17

//---TTGO T-Internet-POE---
//LAN
#define ETH_CLK_MODE_2 ETH_CLOCK_GPIO17_OUT
#define ETH_POWER_PIN_2 -1
#define ETH_TYPE_2 ETH_PHY_LAN8720
#define ETH_ADDR_2 0
#define ETH_MDC_PIN_2 23
#define ETH_MDIO_PIN_2 18
//ZIGBEE
#define RESTART_ZIGBEE_2 12
#define FLASH_ZIGBEE_2 32
#define ZRXD_2 36
#define ZTXD_2 4

//---China-GW---
//LAN
#define ETH_CLK_MODE_3 ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN_3 2
#define ETH_TYPE_3 ETH_PHY_LAN8720
#define ETH_ADDR_3 0
#define ETH_MDC_PIN_3 23
#define ETH_MDIO_PIN_3 18
//ZIGBEE
#define RESTART_ZIGBEE_3 12
#define FLASH_ZIGBEE_3 14
#define ZRXD_3 32
#define ZTXD_3 33

#define PRODUCTION 1
#define FLASH 0

#define MAX_SOCKET_CLIENTS 10
#define BAUD_RATE 38400
#define TCP_LISTEN_PORT 9999

#define ETH_ERROR_TIME 30

#define FORMAT_LITTLEFS_IF_FAILED true

struct ConfigSettingsStruct
{
  bool enableWiFi;
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
  double refreshLogs;
  char hostname[50];
  bool connectedSocket[10];
  int connectedClients;
  bool radioModeWiFi;
  unsigned long socketTime;
  unsigned long disconnectEthTime;
  int board;
  char boardName[50];
  bool emergencyWifi;
  int rstZigbeePin;
  int flashZigbeePin;
  bool mqttEnable;
  char mqttServer[50];
  IPAddress mqttServerIP;
  int mqttPort;
  char mqttUser[50];
  char mqttPass[50];
  char mqttTopic[50];
  //bool mqttRetain;
  int mqttInterval;
  bool mqttDiscovery;
  unsigned long mqttReconnectTime;
  unsigned long mqttHeartbeatTime;
  int tempOffset;
  bool webAuth;
  char webUser[50];
  char webPass[50];
};

struct InfosStruct
{
  char device[8];
  char mac[8];
  char flash[8];
};

typedef CircularBuffer<char, 1024> LogConsoleType;

#define WL_MAC_ADDR_LENGTH 6

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
#endif
