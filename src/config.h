#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <CircularBuffer.h>

#include "Version.h"

// hardware config
#define RESET_ZIGBEE 33 //13
#define FLASH_ZIGBEE 32 //14
#define PRODUCTION 1
#define FLASH 0

#define RXD2 5  //2 //16
#define TXD2 17 //4 //17

#define BAUD_RATE 38400
#define TCP_LISTEN_PORT 9999

#define ETH_ERROR_TIME 30

#define BONJOUR_SUPPORT

#define FORMAT_LITTLEFS_IF_FAILED true

// ma structure config
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
  bool connectedSocket;
  bool radioModeWiFi;
  unsigned long socketTime;
  unsigned long disconnectEthTime;
};

struct InfosStruct
{
  char device[8];
  char mac[8];
  char flash[8];
};

typedef CircularBuffer<char, 1024> LogConsoleType;

#define WL_MAC_ADDR_LENGTH 6

#define DEBUG_ON

#ifdef DEBUG_ON
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
#endif
