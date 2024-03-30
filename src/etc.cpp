#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ETH.h>
#include <CCTools.h>
#include <esp_task_wdt.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "zones.h"
// #include "hw.h"
#include "zb.h"

#include <WireGuard-ESP32.h>
static WireGuard wg;

// extern struct ConfigSettingsStruct ConfigSettings;
extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

extern struct BrdConfigStruct hwConfig;
// extern struct CurrentModesStruct modes;

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

extern struct SysVarsStruct vars;

extern CCTools CCTool;

const char *coordMode = "coordMode";         // coordMode node name ?? not name but text field with mode
const char *prevCoordMode = "prevCoordMode"; // prevCoordMode node name ?? not name but text field with mode

const char *configFileSystem = "/config/system.json";
const char *configFileWifi = "/config/configWifi.json";
const char *configFileEther = "/config/configEther.json";
const char *configFileGeneral = "/config/configGeneral.json";
const char *configFileSecurity = "/config/configSecurity.json";
const char *configFileSerial = "/config/configSerial.json";
const char *configFileMqtt = "/config/configMqtt.json";
const char *configFileWg = "/config/configWg.json";
const char *configFileHw = "/config/configHw.json";

/*
const char *cfgFileHw = "/cfg/hardware.json";
const char *cfgFileGen = "/cfg/general.json";
const char *cfgFileNet = "/cfg/network.json";
const char *cfgFileSec = "/cfg/security.json";
const char *cfgFileSer = "/cfg/serial.json";
const char *cfgFileVpn = "/cfg/vpn.json";
const char *cfgFileMqtt = "/cfg/mqtt.json";
*/

// const char *deviceModel = "XZG";

void getReadableTime(String &readableTime, unsigned long beginTime)
{
  unsigned long currentMillis;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;
  currentMillis = millis() - beginTime;
  seconds = currentMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  readableTime = String(days) + " d ";

  if (hours < 10)
  {
    readableTime += "0";
  }
  readableTime += String(hours) + ":";

  if (minutes < 10)
  {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10)
  {
    readableTime += "0";
  }
  readableTime += String(seconds) + "";
}

float readTemperature(bool clear)
{
  if (clear)
  {
    return (temprature_sens_read() - 32) / 1.8;
  }
  else
  {
    return (temprature_sens_read() - 32) / 1.8 - systemCfg.tempOffset;
  }
}

float getCPUtemp(bool clear)
{
  //DEBUG_PRINTLN(F("getCPUtemp"));
  float CPUtemp = 0.0;
  if (WiFi.getMode() == WIFI_MODE_NULL || WiFi.getMode() == WIFI_OFF)
  {
    //DEBUG_PRINTLN(F("enable wifi to enable temp sensor"));
    WiFi.mode(WIFI_STA); // enable wifi to enable temp sensor
    CPUtemp = readTemperature(clear);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF); // disable wifi
  }
  else
  {
    CPUtemp = readTemperature(clear);
  }
  return CPUtemp;
}

void zigbeeRouterRejoin()
{
  printLogMsg("Router rejoin begin");
  DEBUG_PRINTLN(F("Router rejoin begin"));
  CCTool.routerRejoin();
  printLogMsg("Router in join mode!");
  DEBUG_PRINTLN(F("Router in join mode!"));
}

void zigbeeEnableBSL()
{
  printLogMsg("ZB enable BSL");
  DEBUG_PRINTLN(F("ZB enable BSL"));
  CCTool.enterBSL();
  printLogMsg("Now you can flash CC2652!");
  DEBUG_PRINTLN(F("Now you can flash CC2652!"));
}

void zigbeeRestart()
{
  printLogMsg("ZB RST begin");
  DEBUG_PRINTLN(F("ZB RST begin"));
  CCTool.restart();
  printLogMsg("ZB restart was done");
  DEBUG_PRINTLN(F("ZB restart was done"));
}

void adapterModeUSB()
{
  if (vars.hwUartSelIs)
  {
    printLogMsg("Switched XZG to USB mode");
    DEBUG_PRINTLN(F("Switched XZG to USB mode"));
    if (vars.hwUartSelIs)
    {
      digitalWrite(hwConfig.mist.uartSelPin, 1);
      DEBUG_PRINTLN(F("digitalWrite(hwConfig.mist.uartSelPin, 1) - HIGH"));
    }
    if (vars.hwLedUsbIs)
    {
      digitalWrite(hwConfig.mist.ledUsbPin, 1);
    }
  }
  else
  {
    DEBUG_PRINTLN(F("NO vars.hwUartSelIs. NO mode USB"));
  }
}

void adapterModeLAN()
{
  if (vars.hwUartSelIs)
  {
    printLogMsg("Switched XZG to LAN mode");
    DEBUG_PRINTLN(F("Switched XZG to LAN mode"));
    digitalWrite(hwConfig.mist.uartSelPin, 0);
    DEBUG_PRINTLN(F("digitalWrite(hwConfig.mist.uartSelPin, 0) - LOW"));

    if (vars.hwLedUsbIs)
      digitalWrite(hwConfig.mist.ledUsbPin, 0);
  }
  else
  {
    DEBUG_PRINTLN(F("NO vars.hwUartSelIs. NO mode LAN"));
  }
}

void ledPwrToggle()
{
  if (vars.hwLedPwrIs)
  {
    printLogMsg("BLUE LED has been toggled");
    DEBUG_PRINTLN(F("BLUE LED has been toggled"));
    DEBUG_PRINT(F("pin - "));
    DEBUG_PRINTLN(hwConfig.mist.ledPwrPin);
    digitalWrite(hwConfig.mist.ledPwrPin, !digitalRead(hwConfig.mist.ledPwrPin));
  }
  else
  {
    DEBUG_PRINTLN(F("NO vars.hwLedPwrIs. NO BLUE LED"));
  }
}

void ledUsbToggle()
{
  if (vars.hwLedUsbIs)
  {
    printLogMsg("RED LED has been toggled");
    DEBUG_PRINTLN(F("RED LED has been toggled"));
    digitalWrite(hwConfig.mist.ledUsbPin, !digitalRead(hwConfig.mist.ledUsbPin));
  }
  else
  {
    DEBUG_PRINTLN(F("NO vars.hwLedUsbIs. NO RED LED"));
  }
}

void getDeviceID(char *arr)
{
  uint64_t mac = ESP.getEfuseMac(); // Retrieve the MAC address
  uint8_t a = 0;                    // Initialize variables to store the results
  uint8_t b = 0;

  // Apply XOR operation to each byte of the MAC address to obtain unique values for a and b
  a ^= (mac >> (8 * 0)) & 0xFF;
  a ^= (mac >> (8 * 1)) & 0xFF;
  a ^= (mac >> (8 * 2)) & 0xFF;
  a ^= (mac >> (8 * 3)) & 0xFF;
  b ^= (mac >> (8 * 4)) & 0xFF;
  b ^= (mac >> (8 * 5)) & 0xFF;

  char buf[20];

  // Format a and b into buf as hexadecimal values, ensuring two-digit representation for each
  sprintf(buf, "%02x%02x", a, b);

  // Convert each character in buf to upper case
  for (uint8_t cnt = 0; buf[cnt] != '\0'; cnt++)
  {
    buf[cnt] = toupper(buf[cnt]);
  }

  // Form the final string including the board name and the processed MAC address
  
  //sprintf(arr, "%s-%s", hwConfig.board, buf);
  snprintf(arr, MAX_DEV_ID_LONG, "%s-%s", hwConfig.board, buf);
}

/*
void getDeviceID(char *arr)
{
  uint64_t mac = ESP.getEfuseMac();
  uint8_t a;
  uint8_t b;
  a ^= mac >> 8 * 0;
  a ^= mac >> 8 * 1;
  a ^= mac >> 8 * 2;
  a ^= mac >> 8 * 3;
  b ^= mac >> 8 * 4;
  b ^= mac >> 8 * 5;
  b ^= mac >> 8 * 6;
  b ^= mac >> 8 * 7;

  char buf[20];

  if (a < 16)
  {
    sprintf(buf, "0%x", a);
  }
  else
  {
    sprintf(buf, "%x", a);
  }

  if (b < 16)
  {
    sprintf(buf, "%s0%x", buf, b);
  }
  else
  {
    sprintf(buf, "%s%x", buf, b);
  }

  for (uint8_t cnt = 0; cnt < strlen(buf); cnt++)
  {
    buf[cnt] = toupper(buf[cnt]);
  }

  // char buf[20];
  sprintf(arr, "%s-%s", hwConfig.board, buf);
  // arr = buf;
} */

// void writeDefaultConfig(const char *path, String StringConfig)
// {
//   DEBUG_PRINTLN(path);
//   DEBUG_PRINTLN(F("failed open. try to write defaults"));
//   DEBUG_PRINTLN(StringConfig);

//   DynamicJsonDocument doc(1024);
//   deserializeJson(doc, StringConfig);

//   File configFile = LittleFS.open(path, FILE_WRITE);
//   if (!configFile)
//   {
//     DEBUG_PRINTLN(F("failed write"));
//     //return false;
//   }
//   else
//   {
//     serializeJson(doc, configFile);
//   }
//   configFile.close();
// }

void writeDefaultConfig(const char *path, DynamicJsonDocument &doc)
{
  DEBUG_PRINT(F("Write defaults to "));
  DEBUG_PRINTLN(path);
  serializeJsonPretty(doc, Serial);
  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("Failed Write"));
    // return false;
  }
  else
  {
    serializeJsonPretty(doc, configFile);
  }
  configFile.close();
}

/*String hexToDec(String hexString)
{

  unsigned int decValue = 0;
  int nextInt;

  for (int i = 0; i < hexString.length(); i++)
  {

    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57)
      nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70)
      nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102)
      nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return String(decValue);
}*/

void resetSettings() // to do adapt to Preferences
{
  DEBUG_PRINTLN(F("[resetSettings] Start"));
  if (vars.hwLedPwrIs)
  {
    digitalWrite(hwConfig.mist.ledPwrPin, 1);
  }
  if (vars.hwLedUsbIs)
  {
    digitalWrite(hwConfig.mist.ledUsbPin, 0);
  }
  for (uint8_t i = 0; i < 15; i++)
  {
    delay(200);
    if (vars.hwLedUsbIs)
    {
      digitalWrite(hwConfig.mist.ledUsbPin, !digitalRead(hwConfig.mist.ledUsbPin));
    }
    if (vars.hwLedPwrIs)
    {
      digitalWrite(hwConfig.mist.ledPwrPin, !digitalRead(hwConfig.mist.ledPwrPin));
    }
  }
  DEBUG_PRINTLN(F("[resetSettings] Led blinking done"));
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
  {
    DEBUG_PRINTLN(F("Error with LITTLEFS"));
  }
  LittleFS.remove(configFileSerial);
  LittleFS.remove(configFileSecurity);
  LittleFS.remove(configFileGeneral);
  LittleFS.remove(configFileEther);
  LittleFS.remove(configFileWifi);
  LittleFS.remove(configFileSystem);
  LittleFS.remove(configFileWg);
  LittleFS.remove(configFileHw);
  DEBUG_PRINTLN(F("[resetSettings] Config del done"));
  ESP.restart();
}

void setClock()
{
  configTime(0, 0, systemCfg.ntpServ1, systemCfg.ntpServ2);

  DEBUG_PRINT(F("Waiting for NTP time sync: "));
  int startTryingTime = millis();
  DEBUG_PRINTLN(startTryingTime);
  startTryingTime = startTryingTime / 1000;
  time_t nowSecs = time(nullptr);
  DEBUG_PRINTLN(nowSecs);
  while ((nowSecs - startTryingTime) < 60)
  {
    delay(500);
    DEBUG_PRINT(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  DEBUG_PRINTLN();
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);
  DEBUG_PRINT(F("Current GMT time: "));
  DEBUG_PRINT(asctime(&timeinfo));

  char *zoneToFind = const_cast<char *>("Europe/Kiev");
  if (systemCfg.timeZone)
  {
    zoneToFind = systemCfg.timeZone;
  }
  const char *gmtOffset = getGmtOffsetForZone(zoneToFind);

  String timezone = "EET-2EEST,M3.5.0/3,M10.5.0/4";

  if (gmtOffset != nullptr)
  {
    DEBUG_PRINT(F("GMT Offset for "));
    DEBUG_PRINT(zoneToFind);
    DEBUG_PRINT(F(" is "));
    DEBUG_PRINTLN(gmtOffset);
    timezone = gmtOffset;
    setTimezone(timezone);
  }
  else
  {
    DEBUG_PRINT(F("GMT Offset for "));
    DEBUG_PRINT(zoneToFind);
    DEBUG_PRINTLN(F(" not found."));
  }
}

void setTimezone(String timezone)
{
  DEBUG_PRINTLN(F("Setting Timezone"));
  setenv("TZ", timezone.c_str(), 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
  time_t nowSecs = time(nullptr);
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);

  String timeNow = asctime(&timeinfo);
  timeNow.remove(timeNow.length() - 1);
  DEBUG_PRINT(F("Local time: "));
  DEBUG_PRINTLN(timeNow);
  printLogMsg("Local time: " + timeNow);
}

const char *getGmtOffsetForZone(const char *zone)
{
  for (int i = 0; i < timeZoneCount; i++)
  {
    if (strcmp(zone, timeZones[i].zone) == 0)
    {
      // Zone found, return GMT Offset
      return timeZones[i].gmtOffset;
    }
  }
  // Zone not found
  return nullptr;
}

void ledsScheduler()
{
  DEBUG_PRINTLN(F("LEDS Scheduler"));
}

BrdConfigStruct customConfig;

BrdConfigStruct *findBrdConfig(int searchId = 0)
{
  int brdConfigsSize = sizeof(brdConfigs) / sizeof(brdConfigs[0]);

  bool brdOk = false;

  if (searchId == brdConfigsSize)
  {
    // last config was not successful. so use custom.

    strlcpy(customConfig.board, "Custom", sizeof(customConfig.board));
    return &customConfig;
  }

  int i = searchId;

  if (ETH.begin(brdConfigs[i].eth.addr, brdConfigs[i].eth.pwrPin, brdConfigs[i].eth.mdcPin, brdConfigs[i].eth.mdiPin, brdConfigs[i].eth.phyType, brdConfigs[i].eth.clkMode, brdConfigs[i].eth.pwrAltPin))
  {
    Serial.print("BrdConfig found: ");
    Serial.println(brdConfigs[i].board);
    brdOk = true;
    // zigbee check

    if (brdConfigs[i].zb.rxPin > 0 && brdConfigs[i].zb.txPin > 0 && brdConfigs[i].zb.rstPin > 0 && brdConfigs[i].zb.bslPin > 0)
    {
      DEBUG_PRINTLN(F("Zigbee pins OK. Try to connect..."));

      esp_task_wdt_reset();

      Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, brdConfigs[i].zb.rxPin, brdConfigs[i].zb.txPin); // start zigbee serial

      // CCTool.switchStream(Serial2);

      int BSL_MODE = 0;
      // delay(500);

      if (zbInit(brdConfigs[i].zb.rstPin, brdConfigs[i].zb.bslPin, BSL_MODE))
      {
        DEBUG_PRINTLN(F("Zigbee find - OK"));
        brdOk = true;
      }
      else
      {
        DEBUG_PRINTLN(F("Zigbee find - ERROR"));
        brdOk = false;
      }
    }
  }
  if (brdOk == true)
  {
    return &brdConfigs[i];
  }
  else
  {
    Serial.print("BrdConfig error with: ");
    Serial.println(brdConfigs[i].board);
    // delay(500);

    DynamicJsonDocument config(300);
    config["searchId"] = i + 1;
    writeDefaultConfig(configFileHw, config);

    delay(500);
    DEBUG_PRINTLN(F("Restarting..."));
    ESP.restart();

    return nullptr;
  }
}
/*
ZbConfig *findZbConfig(int ethPwrPin, int ethMdcPin, int ethMdiPin, int ethClkPin, int ethPwrAltPin)
{
  int zbConfigsSize = sizeof(zbConfigs) / sizeof(zbConfigs[0]);

  for (int i = 0; i < zbConfigsSize; i++)
  {
    DEBUG_PRINT(F("Zigbee try "));
    DEBUG_PRINT(zbConfigs[i].rxPin);
    DEBUG_PRINT(F(" "));
    DEBUG_PRINT(zbConfigs[i].txPin);
    DEBUG_PRINT(F(" "));
    DEBUG_PRINT(zbConfigs[i].rstPin);
    DEBUG_PRINT(F(" "));
    DEBUG_PRINTLN(zbConfigs[i].bslPin);

    if (zbConfigs[i].rxPin == ethPwrPin || zbConfigs[i].rxPin == ethMdcPin || zbConfigs[i].rxPin == ethMdiPin || zbConfigs[i].rxPin == ethClkPin || zbConfigs[i].rxPin == ethPwrAltPin)
      continue;
    if (zbConfigs[i].txPin == ethPwrPin || zbConfigs[i].txPin == ethMdcPin || zbConfigs[i].txPin == ethMdiPin || zbConfigs[i].txPin == ethClkPin || zbConfigs[i].txPin == ethPwrAltPin)
      continue;
    if (zbConfigs[i].rstPin == ethPwrPin || zbConfigs[i].rstPin == ethMdcPin || zbConfigs[i].rstPin == ethMdiPin || zbConfigs[i].rstPin == ethClkPin || zbConfigs[i].rstPin == ethPwrAltPin)
      continue;
    if (zbConfigs[i].bslPin == ethPwrPin || zbConfigs[i].bslPin == ethMdcPin || zbConfigs[i].bslPin == ethMdiPin || zbConfigs[i].bslPin == ethClkPin || zbConfigs[i].bslPin == ethPwrAltPin)
      continue;

    DEBUG_PRINTLN(F("Zigbee no conflict with ETH"));

    //Serial2.end();
    //DEBUG_PRINTLN(F("Serial2.end"));

    Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, zbConfigs[i].rxPin, zbConfigs[i].txPin); // start zigbee serial
    DEBUG_PRINTLN(F("Serial2.begin"));

    CCTool.switchStream(Serial2);

    int BSL_MODE = 0;
    delay(500);
    if (zbInit(zbConfigs[i].rstPin, zbConfigs[i].bslPin, BSL_MODE))
    {
      DEBUG_PRINTLN(F("Zigbee find - OK"));
      return &zbConfigs[i];
    }
    else
    {
      DEBUG_PRINTLN(F("Zigbee find - ERROR"));
    }
  }

  return nullptr;
}
*/

void wgBegin()
{
  if (!wg.is_initialized())
  {
    // printLogMsg(String("Initializing WireGuard interface..."));
    auto subnet = IPAddress(255, 255, 255, 255);
    auto gateway = IPAddress(0, 0, 0, 0);
    auto localport = 50000;
    if (!wg.begin(
            vpnCfg.wgLocalIP,
            subnet,
            localport,
            gateway,
            vpnCfg.wgLocalPrivKey,
            vpnCfg.wgEndAddr,
            vpnCfg.wgEndPubKey,
            vpnCfg.wgEndPort))
    {
      printLogMsg(String("Failed to initialize WG"));
      vars.vpnWgInit = false;
    }
    else
    {
      printLogMsg(String("WG was initialized"));
      vars.vpnWgInit = true;
    }
  }
}

void wgLoop()
{
  String tag = "WG";
  uint16_t localport = 50000;
  //IPAddress ip = ;
  
  int checkPeriod = 5; // to do vpnCfg.checkTime; 
  ip_addr_t lwip_ip;

  lwip_ip.u_addr.ip4.addr = static_cast<uint32_t>(vpnCfg.wgLocalIP);

  if (wg.is_initialized())
  {
    if (vars.vpnWgCheckTime == 0)
    {
      vars.vpnWgCheckTime = millis() + 1000 * checkPeriod; 
    }
    else
    {
      if (vars.vpnWgCheckTime <= millis())
      {
        //LOGI(tag, "check");
        vars.vpnWgCheckTime = millis() + 1000 * checkPeriod;
        if (wg.is_peer_up(&lwip_ip, &localport))
        {
          vars.vpnWgPeerIp = (lwip_ip.u_addr.ip4.addr);
          if (!vars.vpnWgConnect)
          {
            LOGI(tag, "Peer with IP %s connect", vars.vpnWgPeerIp.toString().c_str());
          }
          vars.vpnWgConnect = true;
        }
        else
        {
          if (vars.vpnWgConnect)
          {
            LOGI(tag, "Peer disconnect");
          }
          vars.vpnWgPeerIp.clear();
          vars.vpnWgConnect = false;
        }
      }
    }
  }
  else
  {
    vars.vpnWgInit = false;
  }
}