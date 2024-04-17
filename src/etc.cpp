#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ETH.h>
#include <CCTools.h>
#include <esp_task_wdt.h>
#include <CronAlarms.h>
// #include <Husarnet.h> //not available now

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "const/zones.h"
// #include "const/hw.h"
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

extern LEDControl ledControl;

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
const char *configFileHw = "/configHw.json";

#include "mbedtls/md.h"

String sha1(String payloadStr)
{
  const char *payload = payloadStr.c_str();

  int size = 20;

  byte shaResult[size];

  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA1;

  const size_t payloadLength = strlen(payload);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)payload, payloadLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  String hashStr = "";

  for (uint16_t i = 0; i < size; i++)
  {
    String hex = String(shaResult[i], HEX);
    if (hex.length() < 2)
    {
      hex = "0" + hex;
    }
    hashStr += hex;
  }

  return hashStr;
}

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
  // DEBUG_PRINTLN(F("getCPUtemp"));
  float CPUtemp = 0.0;
  if (WiFi.getMode() == WIFI_MODE_NULL || WiFi.getMode() == WIFI_OFF)
  {
    // DEBUG_PRINTLN(F("enable wifi to enable temp sensor"));
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
    ledControl.modeLED.mode = LED_ON;
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
    ledControl.modeLED.mode = LED_OFF;
  }
  else
  {
    DEBUG_PRINTLN(F("NO vars.hwUartSelIs. NO mode LAN"));
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

  char buf[MAX_DEV_ID_LONG];

  // Format a and b into buf as hexadecimal values, ensuring two-digit representation for each
  sprintf(buf, "%02x%02x", a, b);

  // Convert each character in buf to upper case
  for (uint8_t cnt = 0; buf[cnt] != '\0'; cnt++)
  {
    buf[cnt] = toupper(buf[cnt]);
  }

  // Form the final string including the board name and the processed MAC address

  // sprintf(arr, "%s-%s", hwConfig.board, buf);

  String devicePref = "XZG"; // hwConfig.board
  snprintf(arr, MAX_DEV_ID_LONG, "%s-%s", devicePref, buf);
}

void writeDefaultConfig(const char *path, DynamicJsonDocument &doc)
{
  DEBUG_PRINT(F("Write defaults to "));
  DEBUG_PRINTLN(path);
  serializeJsonPretty(doc, Serial);
  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("Failed Write"));
    if (LittleFS.mkdir(path))
    {
      DEBUG_PRINTLN(F("Config dir created"));
      delay(500);
      ESP.restart();
    }
    else
    {
      DEBUG_PRINTLN(F("mkdir failed"));
    }
    // return false;
  }
  else
  {
    serializeJsonPretty(doc, configFile);
  }
  configFile.close();
}

void factoryReset()
{
  String tag = "FactoryReset";

  LOGD("start");

  ledControl.powerLED.mode = LED_FLASH_3Hz;
  ledControl.modeLED.mode = LED_FLASH_3Hz;

  for (uint8_t i = 0; i < 5; i++)
  {
    LOGD("%d, sec", 5 - i);
    delay(1000);
  }

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
  {
    LOGD("Error with LITTLEFS");
  }
  LittleFS.remove(configFileSerial);
  LittleFS.remove(configFileSecurity);
  LittleFS.remove(configFileGeneral);
  LittleFS.remove(configFileEther);
  LittleFS.remove(configFileWifi);
  LittleFS.remove(configFileSystem);
  LittleFS.remove(configFileWg);
  LittleFS.remove(configFileHw);
  LittleFS.remove(configFileMqtt);
  LOGD("FS Done");
  eraseNVS();
  LOGD("NVS Done");

  ledControl.powerLED.mode = LED_OFF;
  ledControl.modeLED.mode = LED_OFF;
  delay(500);
  ESP.restart();
}

void setClock(void *pvParameters)
{
  String tag = "clock";
  configTime(0, 0, systemCfg.ntpServ1, systemCfg.ntpServ2);

  const time_t targetTime = 946684800; // 946684800 - is 01.01.2000 in timestamp

  LOGD("Waiting for NTP time sync: ");
  unsigned long startTryingTime = millis();

  time_t nowSecs = time(nullptr);

  // over 01.01.2000 or longer than 5 minutes
  while ((nowSecs < targetTime) && ((millis() - startTryingTime) < 300000))
  {
    delay(500);
    DEBUG_PRINT(F("."));
    yield();
    nowSecs = time(nullptr);
  }
  DEBUG_PRINTLN();

  struct tm timeinfo;
  if (localtime_r(&nowSecs, &timeinfo))
  {
    LOGD("Current GMT time: %s", String(asctime(&timeinfo)).c_str());

    char *zoneToFind = const_cast<char *>("Europe/Kiev");
    if (systemCfg.timeZone)
    {
      zoneToFind = systemCfg.timeZone;
    }
    const char *gmtOffset = getGmtOffsetForZone(zoneToFind);

    String timezone = "EET-2EEST,M3.5.0/3,M10.5.0/4";

    if (gmtOffset != nullptr)
    {
      LOGD("GMT Offset for %s is %s", zoneToFind, gmtOffset);
      timezone = gmtOffset;
      setTimezone(timezone);
    }
    else
    {
      LOGD("GMT Offset for %s not found.", zoneToFind);
    }

    setupCron();
  }
  else
  {
    LOGD("Failed to get time from NTP server.");
  }
  vTaskDelete(NULL);
}

void setLedsDisable(bool all)
{
  if (vars.hwLedPwrIs || vars.hwLedUsbIs)
  {
    LOGD("setLedsDisable", "%s", String(all));
    if (all)
    {
      ledControl.powerLED.active = false;
      ledControl.modeLED.active = false;
    }
    else
    {
      ledControl.powerLED.active = !systemCfg.disableLedPwr;
      ledControl.modeLED.active = !systemCfg.disableLedUSB;
    }
  }
}

void nmActivate()
{
  LOGD("NM", "start");
  setLedsDisable(true);
}

void nmDeactivate()
{
  LOGD("NM", "end");
  setLedsDisable();
}

void setupCron()
{
  // Cron.create(const_cast<char *>("0 */1 * * * *"), cronTest, false);

  if (systemCfg.nmEnable)
  {

    time_t nowSecs = time(nullptr);
    struct tm timeinfo;
    localtime_r(&nowSecs, &timeinfo);
    int currentTimeInMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    int sTargetHour, sTargetMinute, startTimeInMinutes;
    int eTargetHour, eTargetMinute, endTimeInMinutes;

    sscanf(systemCfg.nmStart, "%d:%d", &sTargetHour, &sTargetMinute);
    startTimeInMinutes = sTargetHour * 60 + sTargetMinute;

    sscanf(systemCfg.nmEnd, "%d:%d", &eTargetHour, &eTargetMinute);
    endTimeInMinutes = eTargetHour * 60 + eTargetMinute;

    if (startTimeInMinutes <= endTimeInMinutes)
    {
      if (currentTimeInMinutes >= startTimeInMinutes && currentTimeInMinutes < endTimeInMinutes)
      {
        nmActivate();
      }
      else
      {
        nmDeactivate();
      }
    }
    else
    {
      if (currentTimeInMinutes >= startTimeInMinutes || currentTimeInMinutes < endTimeInMinutes)
      {
        nmActivate();
      }
      else
      {
        nmDeactivate();
      }
    }

    char startCron[30];
    char endCron[30];
    strcpy(startCron, convertTimeToCron(String(systemCfg.nmStart)));
    strcpy(endCron, convertTimeToCron(String(systemCfg.nmEnd)));
    LOGD("cron", "NM start %s", startCron);
    LOGD("cron", "NM end %s", endCron);

    Cron.create(const_cast<char *>(startCron), nmActivate, false);
    Cron.create(const_cast<char *>(endCron), nmDeactivate, false);
  }
}

void setTimezone(String timezone)
{
  String tag = "TZ";
  LOGD("Setting Timezone");
  setenv("TZ", timezone.c_str(), 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
  time_t nowSecs = time(nullptr);
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);

  String timeNow = asctime(&timeinfo);
  timeNow.remove(timeNow.length() - 1);
  LOGD("Local time: %s", timeNow.c_str());
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

String getTime()
{
  time_t nowSecs = time(nullptr);
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);

  String timeNow = asctime(&timeinfo);
  timeNow.remove(timeNow.length() - 1);
  return timeNow;
}

char *convertTimeToCron(const String &time)
{
  static char formattedTime[16];
  int hours, minutes;

  // Использование метода toCharArray() для получения C-строки из объекта String
  char timeArray[6]; // Достаточно места для "ЧЧ:ММ\0"
  time.toCharArray(timeArray, sizeof(timeArray));

  // Разбор времени
  sscanf(timeArray, "%d:%d", &hours, &minutes);

  // Формирование cron-строки
  snprintf(formattedTime, sizeof(formattedTime), "0 %d %d * * *", minutes, hours);

  return formattedTime;
}

void ledsScheduler()
{
  DEBUG_PRINTLN(F("LEDS Scheduler"));
}

BrdConfigStruct customConfig;

BrdConfigStruct *findBrdConfig(int searchId = 0)
{
  String tag = "BRD";
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
    LOGD("Config found: %s", brdConfigs[i].board);
    brdOk = true;

    if (brdConfigs[i].zb.rxPin > 0 && brdConfigs[i].zb.txPin > 0 && brdConfigs[i].zb.rstPin > 0 && brdConfigs[i].zb.bslPin > 0)
    {
      LOGD("Zigbee pins OK. Try to connect...");
      esp_task_wdt_reset();
      Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, brdConfigs[i].zb.rxPin, brdConfigs[i].zb.txPin); // start zigbee serial

      // CCTool.switchStream(Serial2);
      int BSL_MODE = 0;
      if (zbInit(brdConfigs[i].zb.rstPin, brdConfigs[i].zb.bslPin, BSL_MODE))
      {
        LOGD("Zigbee find - OK");
        brdOk = true;
      }
      else
      {
        LOGP("Zigbee find - ERROR");
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
    LOGP("Config error with: %s", brdConfigs[i].board);
    DynamicJsonDocument config(300);
    config["searchId"] = i + 1;
    writeDefaultConfig(configFileHw, config);

    LOGD("Restarting...");

    delay(500);
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
  // IPAddress ip = ;

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
        // LOGD("check");
        vars.vpnWgCheckTime = millis() + 1000 * checkPeriod;
        if (wg.is_peer_up(&lwip_ip, &localport))
        {
          vars.vpnWgPeerIp = (lwip_ip.u_addr.ip4.addr);
          if (!vars.vpnWgConnect)
          {
            LOGD("Peer with IP %s connect", vars.vpnWgPeerIp.toString().c_str());
          }
          vars.vpnWgConnect = true;
        }
        else
        {
          if (vars.vpnWgConnect)
          {
            LOGD("Peer disconnect");
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

/* //not available now
void hnBegin()
{
  Husarnet.selfHostedSetup(vpnCfg.hnDashUrl);
  Husarnet.join(vpnCfg.hnJoinCode, vpnCfg.hnHostName);
  Husarnet.start();
}
*/

void ledTask(void *parameter)
{
  LEDSettings *led = (LEDSettings *)parameter;
  TickType_t lastWakeTime = xTaskGetTickCount();
  int previousMode = LED_OFF;

  // String tag = "ledTask";
  while (1)
  {
    // LOGD("%d | led %s | m %d", millis(), led->name, led->mode);
    if (led->pin == -1)
      continue;
    if (!led->active)
    {
      digitalWrite(led->pin, LOW);
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
      continue;
    }

    switch (led->mode)
    {
    case LED_OFF:
      previousMode = led->mode;
      digitalWrite(led->pin, LOW);
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
      break;
    case LED_ON:
      previousMode = led->mode;
      digitalWrite(led->pin, HIGH);
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
      break;
    case LED_TOGGLE:
      if (digitalRead(led->pin) == LOW)
      {
        digitalWrite(led->pin, HIGH);
        led->mode = LED_ON;
      }
      else
      {
        digitalWrite(led->pin, LOW);
        led->mode = LED_OFF;
      }
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(500));
      break;
    case LED_BLINK_5T:
      for (int j = 0; j < 5; j++)
      {
        digitalWrite(led->pin, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(led->pin, LOW);
        vTaskDelay(pdMS_TO_TICKS(500));
      }
      led->mode = static_cast<LEDMode>(previousMode);
      break;
    case LED_BLINK_1T:
      digitalWrite(led->pin, HIGH);
      vTaskDelay(pdMS_TO_TICKS(500));
      digitalWrite(led->pin, LOW);
      vTaskDelay(pdMS_TO_TICKS(500));
      led->mode = static_cast<LEDMode>(previousMode);
      break;
    case LED_BLINK_1Hz:
    case LED_FLASH_1Hz:
      previousMode = led->mode;
      digitalWrite(led->pin, (led->mode == LED_BLINK_1Hz) ? HIGH : LOW);
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(800));
      digitalWrite(led->pin, (led->mode == LED_BLINK_1Hz) ? LOW : HIGH);
      vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(200));
      break;
    case LED_BLINK_3Hz:
    case LED_FLASH_3Hz:
      previousMode = led->mode;
      for (int j = 0; j < 3; j++)
      {
        digitalWrite(led->pin, (led->mode == LED_BLINK_3Hz) ? HIGH : LOW);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(267));
        digitalWrite(led->pin, (led->mode == LED_BLINK_3Hz) ? LOW : HIGH);
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100));
      }
      break;
    }
  }
}