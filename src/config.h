#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>
#include <CircularBuffer.h>
#include "version.h"

//ESP32 PINS TO CONTROL LAN8720
#define ETH_CLK_MODE_1 ETH_CLOCK_GPIO0_IN
#define ETH_POWER_PIN_1 16
#define ETH_TYPE_1 ETH_PHY_LAN8720
#define ETH_ADDR_1 1
#define ETH_MDC_PIN_1 23
#define ETH_MDIO_PIN_1 18
//ESP32 PINS TO CONTROL CC2652P
#define CC2652P_RST 33
#define CC2652P_FLSH 32
#define CC2652P_RXD 5
#define CC2652P_TXD 17
//ESP32 PINS TO CONTROL LEDs, BUTTON AND SWITCH
#define LED_BLUE 12
#define LED_YELLOW 14
#define BTN 35
#define MODE_SWITCH 4
#define DEBOUNCE_TIME 70 


#define PRODUCTION 1
#define FLASH 0

#define MAX_SOCKET_CLIENTS 5
#define BAUD_RATE 38400
#define TCP_LISTEN_PORT 9999

#define ETH_ERROR_TIME 30

#define FORMAT_LITTLEFS_IF_FAILED true

struct ConfigSettingsStruct
{
  bool workMode; //0 - LAN, 1 - USB
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
  int refreshLogs;
  char hostname[50];
  bool connectedSocket[10];
  int connectedClients;
  bool wifiModeAP;
  unsigned long socketTime;
  unsigned long disconnectEthTime;
  int board;
  char boardName[50];
  bool emergencyWifi;
  int tempOffset;
  bool webAuth;
  char webUser[50];
  char webPass[50];
  bool disableEmerg;
  bool usbMode;
  bool disableLedBlue;
  bool disableLedYellow;
  int wifiRetries;
  bool disablePingCtrl;
  int restarts;
  unsigned long wifiAPenblTime;
  bool disableLeds;
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
