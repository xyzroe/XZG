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
#include "main.h"

#include <WireGuard-ESP32.h>
static WireGuard wg;

extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];
extern EthConfig ethConfigs[ETH_CFG_CNT];
extern ZbConfig zbConfigs[ZB_CFG_CNT];
extern MistConfig mistConfigs[MIST_CFG_CNT];

extern struct ThisConfigStruct hwConfig;

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

extern struct SysVarsStruct vars;

extern LEDControl ledControl;

extern CCTools CCTool;

const char *coordMode = "coordMode"; // coordMode node name ?? not name but text field with mode
// const char *prevCoordMode = "prevCoordMode"; // prevCoordMode node name ?? not name but text field with mode

/*const char *configFileSystem = "/config/system.json";
const char *configFileWifi = "/config/configWifi.json";
const char *configFileEther = "/config/configEther.json";
const char *configFileGeneral = "/config/configGeneral.json";
const char *configFileSecurity = "/config/configSecurity.json";
const char *configFileSerial = "/config/configSerial.json";
const char *configFileMqtt = "/config/configMqtt.json";
const char *configFileWg = "/config/configWg.json";*/
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

// #include <OneWire.h>
// #include <DallasTemperature.h>

// OneWire *oneWire = nullptr;
// DallasTemperature *sensor = nullptr;
#include "DS18B20.h"

OneWire *oneWire = nullptr;
DS18B20 *sensor = nullptr;

int check1wire()
{
  int pin = -1;
  vars.oneWireIs = false;
  if (hwConfig.eth.mdcPin != 33 && hwConfig.eth.mdiPin != 33 && hwConfig.eth.pwrPin != 33)
  {
    if (hwConfig.zb.rxPin != 33 && hwConfig.zb.txPin != 33 && hwConfig.zb.bslPin != 33 && hwConfig.zb.rstPin != 33)
    {
      if (hwConfig.mist.btnPin != 33 && hwConfig.mist.uartSelPin != 33 && hwConfig.mist.ledModePin != 33 && hwConfig.mist.ledPwrPin != 33)
      {
        pin = 33;
        vars.oneWireIs = true;
      }
    }
  }
  return pin;
}

void setup1wire(int pin)
{

  if (pin > 0)
  {
    if (oneWire != nullptr)
    {
      delete oneWire;
    }
    if (sensor != nullptr)
    {
      delete sensor;
    }

    oneWire = new OneWire(pin);
    sensor = new DS18B20(oneWire);

    sensor->begin();
    sensor->setResolution(10);

#ifdef DEBUG
    uint32_t start, stop;

    start = micros();
    sensor->requestTemperatures();

    int n = 0;
    //  wait until sensor ready, do some counting for fun.
    while (!sensor->isConversionComplete())
      n++;

    stop = micros();
    LOGD("Convert %lu\t%d", stop - start, n);

    // delay(100);
    start = micros();
    float f = sensor->getTempC();
    stop = micros();

    LOGD("getTemp %lu\t%.2f", stop - start, f);
#endif

    get1wire();
  }
}

float get1wire()
{

  if (sensor == nullptr)
  {
    LOGW("1w not init");
    vars.oneWireIs = false;
    return -127.0;
  }
  if (millis() - vars.last1wAsk > 5000)
  {
    sensor->requestTemperatures();
    vars.temp1w = sensor->getTempC();
    LOGD("Temp is %f", vars.temp1w);
    vars.last1wAsk = millis();
  }

  if (vars.temp1w == -127)
  {
    vars.oneWireIs = false;
    LOGW("1w not connected");
  }

  return vars.temp1w;
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
  float CPUtemp = 0.0;
  if (WiFi.getMode() == WIFI_MODE_NULL || WiFi.getMode() == WIFI_OFF)
  {
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
  CCTool.routerRejoin();
  printLogMsg("Router in join mode!");
}

void zigbeeEnableBSL()
{
  printLogMsg("ZB enable BSL");
  CCTool.enterBSL();
  printLogMsg("Now you can flash CC2652!");
  if (systemCfg.workMode == WORK_MODE_USB)
  {
    Serial.updateBaudRate(500000);
    Serial2.updateBaudRate(500000);
  }
}

void zigbeeRestart()
{
  printLogMsg("ZB RST begin");
  CCTool.restart();
  printLogMsg("ZB restart was done");
  if (systemCfg.workMode == WORK_MODE_USB)
  {
    Serial.updateBaudRate(systemCfg.serialSpeed);
    Serial2.updateBaudRate(systemCfg.serialSpeed);
  }
}

void usbModeSet(usbMode mode)
{
  // if (vars.hwUartSelIs)
  //{
  // String modeStr = (mode == ZIGBEE) ? "ZIGBEE" : "ESP";
  bool pinValue = (mode == ZIGBEE) ? HIGH : LOW;
  // String msg = "Switched USB to " + modeStr + "";
  // printLogMsg(msg);

  if (mode == ZIGBEE)
  {
    Serial.updateBaudRate(systemCfg.serialSpeed);
  }
  else
  {
    Serial.updateBaudRate(115200);
  }
  // digitalWrite(hwConfig.mist.uartSelPin, pinValue);
  if (pinValue)
  {
    ledControl.modeLED.mode = LED_ON;
  }
  else
  {
    ledControl.modeLED.mode = LED_OFF;
  }
  //}
  // else
  //{
  //  LOGD("NO vars.hwUartSelIs");
  //}
}

void writeDefaultDeviceId(char *arr, bool ethernet)
{
    char id_str[MAX_DEV_ID_LONG] = "XZG-";
    const size_t id_str_len = strlen(id_str);
    union MacAddress {
        uint8_t bytes[8];
        uint64_t value;
    } mac = {};

    if (ethernet) {
        ETH.macAddress(mac.bytes);
    }
    else {
        mac.value = ESP.getEfuseMac();
    }
    snprintf(&id_str[id_str_len],
             MAX_DEV_ID_LONG - id_str_len,
             "%02X%02X",
             mac.bytes[4], mac.bytes[5]);
    memcpy(arr, id_str, MAX_DEV_ID_LONG);
}

/*void writeDefaultConfig(const char *path, DynamicJsonDocument &doc)
{
  LOGD("Write defaults to %s", path);
  serializeJsonPretty(doc, Serial);
  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    LOGD("Failed Write");
    if (LittleFS.mkdir(path))
    {
      LOGD("Config dir created");
      estartDevice();
    }
    else
    {
      LOGD("mkdir failed");
    }
    // return false;
  }
  else
  {
    serializeJsonPretty(doc, configFile);
  }
  configFile.close();
}*/

void factoryReset()
{

  LOGD("start");

  ledControl.powerLED.mode = LED_FLASH_3Hz;
  ledControl.modeLED.mode = LED_FLASH_3Hz;

  /*for (uint8_t i = 0; i < TIMEOUT_FACTORY_RESET; i++)
  {
    LOGD("%d, sec", TIMEOUT_FACTORY_RESET - i);
    delay(1000);
  }*/

  /*LittleFS.format();
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10)) // change to format anyway
  {
    LOGD("Error with LITTLEFS");
  }*/

  /*LittleFS.remove(configFileSerial);
  LittleFS.remove(configFileSecurity);
  LittleFS.remove(configFileGeneral);
  LittleFS.remove(configFileEther);
  LittleFS.remove(configFileWifi);
  LittleFS.remove(configFileSystem);
  LittleFS.remove(configFileMqtt);
  LittleFS.remove(configFileWg);*/
  // LittleFS.remove(configFileHw);

  // LOGD("FS Done");

  eraseNVS();
  LOGD("NVS Done");

  ledControl.powerLED.mode = LED_OFF;
  ledControl.modeLED.mode = LED_OFF;
  restartDevice();
}

void setClock(void *pvParameters)
{
  // checkDNS();
  configTime(0, 0, systemCfg.ntpServ1, systemCfg.ntpServ2);

  const time_t targetTime = 946684800; // 946684800 - is 01.01.2000 in timestamp

  LOGD("Waiting for NTP time sync");
  unsigned long startTryingTime = millis();

  time_t nowSecs = time(nullptr);

  // over 01.01.2000 or longer than 5 minutes
  while ((nowSecs < targetTime) && ((millis() - startTryingTime) < 300000))
  {
    delay(500);
    yield();
    nowSecs = time(nullptr);
  }

  struct tm timeinfo;
  if (localtime_r(&nowSecs, &timeinfo))
  {
    // LOGD("Current GMT time: %s", String(asctime(&timeinfo)).c_str());

    char *zoneToFind = const_cast<char *>(NTP_TIME_ZONE);
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
    LOGD("[setLedsDisable] %s", String(all).c_str());
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
  LOGD("start");
  setLedsDisable(true);
}

void nmDeactivate()
{
  LOGD("end");
  setLedsDisable();
}
/*
bool checkDNS(bool setup)
{
  const char *wifiKey = "WiFi";
  const char *ethKey = "ETH";
  const char *savedKey = "Saved";
  const char *restoredKey = "Restored";
  const char *dnsTagKey = "[DNS]";
  char buffer[100];

  if (networkCfg.wifiEnable)
  {
    IPAddress currentWifiDNS = WiFi.dnsIP();
    if (currentWifiDNS != vars.savedWifiDNS)
    {
      char dnsStrW[16];
      snprintf(dnsStrW, sizeof(dnsStrW), "%u.%u.%u.%u", currentWifiDNS[0], currentWifiDNS[1], currentWifiDNS[2], currentWifiDNS[3]);

      int lastDot = -1;
      for (int i = 0; dnsStrW[i] != '\0'; i++)
      {
        if (dnsStrW[i] == '.')
        {
          lastDot = i;
        }
      }

      int fourthPartW = atoi(dnsStrW + lastDot + 1);

      if (setup && fourthPartW != 0)
      {
        vars.savedWifiDNS = currentWifiDNS;
        snprintf(buffer, sizeof(buffer), "%s %s %s - %s", dnsTagKey, savedKey, wifiKey, dnsStrW);
        printLogMsg(buffer);
      }
      else
      {
        if (vars.savedWifiDNS)
        {
          WiFi.config(WiFi.localIP(), WiFi.gatewayIP(), WiFi.subnetMask(), vars.savedWifiDNS);
          snprintf(buffer, sizeof(buffer), "%s %s %s - %s", dnsTagKey, restoredKey, wifiKey, vars.savedWifiDNS.toString().c_str());
          printLogMsg(buffer);
        }
      }
    }
  }

  if (networkCfg.ethEnable)
  {
    IPAddress currentEthDNS = ETH.dnsIP();
    if (currentEthDNS != vars.savedEthDNS)
    {
      char dnsStrE[16];
      snprintf(dnsStrE, sizeof(dnsStrE), "%u.%u.%u.%u", currentEthDNS[0], currentEthDNS[1], currentEthDNS[2], currentEthDNS[3]);

      int lastDot = -1;
      for (int i = 0; dnsStrE[i] != '\0'; i++)
      {
        if (dnsStrE[i] == '.')
        {
          lastDot = i;
        }
      }

      int fourthPartE = atoi(dnsStrE + lastDot + 1);

      if (setup && fourthPartE != 0)
      {
        vars.savedEthDNS = currentEthDNS;
        snprintf(buffer, sizeof(buffer), "%s %s %s - %s", dnsTagKey, savedKey, ethKey, dnsStrE);
        printLogMsg(buffer);
      }
      else
      {
        if (vars.savedEthDNS)
        {
          ETH.config(ETH.localIP(), ETH.gatewayIP(), ETH.subnetMask(), vars.savedEthDNS);
          snprintf(buffer, sizeof(buffer), "%s %s %s - %s", dnsTagKey, restoredKey, ethKey, vars.savedEthDNS.toString().c_str());
          printLogMsg(buffer);
        }
      }
    }
  }
  return true;
}
*/

/*void reCheckDNS()
{
  checkDNS();
}*/

void setupCron()
{
  // Cron.create(const_cast<char *>("30 */1 * * * *"), reCheckDNS, false);

  // const String time = systemCfg.updCheckTime;
  static char formattedTime[16];
  int seconds, hours, minutes;

  String wday = systemCfg.updCheckDay;

  if (wday != "0")
  {
    seconds = random(1, 59);

    // char timeArray[6];
    // String(systemCfg.updCheckTime).toCharArray(timeArray, sizeof(timeArray));

    sscanf(systemCfg.updCheckTime, "%d:%d", &hours, &minutes);

    snprintf(formattedTime, sizeof(formattedTime), "%d %d %d * * %s", seconds, minutes, hours, wday.c_str());

    // LOGD("UPD cron %s", String(formattedTime));
    printLogMsg("[UPD_CHK] cron " + String(formattedTime));

    Cron.create(const_cast<char *>(formattedTime), checkUpdateAvail, false); // 0 0 */6 * * *
  }
  else
  {
    printLogMsg("[UPD_CHK] cron disabled");
  }

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
    LOGD("[cron] NM start %s", const_cast<char *>(startCron));
    LOGD("[cron] NM end %s", const_cast<char *>(endCron));

    Cron.create(const_cast<char *>(startCron), nmActivate, false);
    Cron.create(const_cast<char *>(endCron), nmDeactivate, false);
  }
}

void setTimezone(String timezone)
{
  // LOGD("Setting Timezone");
  setenv("TZ", timezone.c_str(), 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
  time_t nowSecs = time(nullptr);
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);

  String timeNow = asctime(&timeinfo);
  timeNow.remove(timeNow.length() - 1);
  printLogMsg("[Time] " + timeNow);
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

  char timeArray[6];
  time.toCharArray(timeArray, sizeof(timeArray));

  sscanf(timeArray, "%d:%d", &hours, &minutes);

  snprintf(formattedTime, sizeof(formattedTime), "0 %d %d * * *", minutes, hours);

  return formattedTime;
}

ThisConfigStruct *findBrdConfig(int searchId = 0)
{

  bool ethOk = false;
  bool btnOk = false;
  bool zbOk = false;

  static ThisConfigStruct bestConfig;
  bestConfig.eth = {.addr = -1, .pwrPin = -1, .mdcPin = -1, .mdiPin = -1, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO17_OUT}; // .pwrAltPin = -1};
  bestConfig.zb = {.txPin = -1, .rxPin = -1, .rstPin = -1, .bslPin = -1};
  memset(&bestConfig.mist, -1, sizeof(bestConfig.mist));
  strlcpy(bestConfig.board, "Unknown", sizeof(bestConfig.board));

  for (int brdIdx = searchId; brdIdx < BOARD_CFG_CNT; ++brdIdx)
  {
    int ethIdx = brdConfigs[brdIdx].ethConfigIndex;
    int zbIdx = brdConfigs[brdIdx].zbConfigIndex;
    int mistIdx = brdConfigs[brdIdx].mistConfigIndex;

    LOGI("Try brd: %d - %s", brdIdx, brdConfigs[brdIdx].board);

    if (ethIdx == -1)
    {
      ethOk = true;
      LOGD("NO ethernet OK: %d", ethIdx);
    } // egin(ETH_PHY_TYPE, ETH_PHY_ADDR, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_PHY_POWER, ETH_CLK_MODE);

#ifdef TASMOTA_PLATFORM
    else if (ETH.begin(ethConfigs[ethIdx].phyType, ethConfigs[ethIdx].addr, ethConfigs[ethIdx].mdcPin, ethConfigs[ethIdx].mdiPin, ethConfigs[ethIdx].pwrPin, ethConfigs[ethIdx].clkMode)) // ethConfigs[ethIdx].pwrAltPin))
#else
    else if (ETH.begin(ethConfigs[ethIdx].addr, ethConfigs[ethIdx].pwrPin, ethConfigs[ethIdx].mdcPin, ethConfigs[ethIdx].mdiPin, ethConfigs[ethIdx].phyType, ethConfigs[ethIdx].clkMode)) // ethConfigs[ethIdx].pwrAltPin))
#endif
    {
      ethOk = true;
      bestConfig.eth = ethConfigs[ethIdx];
      LOGD("Ethernet config OK: %d", ethIdx);
    }

    if (mistConfigs[mistIdx].btnPin > 0)
    {
      pinMode(mistConfigs[mistIdx].btnPin, INPUT);
      int press = 0;
      for (int y = 0; y < 10; y++)
      {
        int state = digitalRead(mistConfigs[mistIdx].btnPin);
        if (state != mistConfigs[mistIdx].btnPlr)
        {
          press++;
        }
        delay(100);
      }
      btnOk = (press <= 5);

      if (!btnOk)
      {
        LOGD("Button pin ERROR");
        ethOk = false;
      }
      else
      {
        LOGD("Button pin OK");
      }
    }
    else
    {
      btnOk = true;
    }

    if (btnOk)
    {
      LOGD("Trying Zigbee: %d", zbIdx);
      esp_task_wdt_reset();
      Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, zbConfigs[zbIdx].rxPin, zbConfigs[zbIdx].txPin);

      int BSL_PIN_MODE = 0;
      if (CCTool.begin(zbConfigs[zbIdx].rstPin, zbConfigs[zbIdx].bslPin, BSL_PIN_MODE))
      {
        if (CCTool.detectChipInfo())
        {
          zbOk = true;
          LOGD("Zigbee config OK: %d", zbIdx);

          bestConfig.zb = zbConfigs[zbIdx];
          bestConfig.mist = mistConfigs[mistIdx];

          bool multiCfg = false;
          int brdNewId = -1;
          for (int brdNewIdx = 0; brdNewIdx < BOARD_CFG_CNT; brdNewIdx++)
          {
            LOGD("%d %d", brdIdx, brdNewIdx);
            if (brdIdx != brdNewIdx && brdConfigs[brdNewIdx].ethConfigIndex == ethIdx && brdConfigs[brdNewIdx].zbConfigIndex == zbIdx && brdConfigs[brdNewIdx].mistConfigIndex == mistIdx)
            {
              multiCfg = true;
              brdNewId = brdNewIdx;
              break;
            }
          }
          const char *nameBrd;
          if (!multiCfg)
          {
            nameBrd = brdConfigs[brdIdx].board;
            strlcpy(bestConfig.board, nameBrd, sizeof(bestConfig.board));
          }
          else
          {
            String nameBrdStr = ("Multi_" + String(brdNewId));
            nameBrd = nameBrdStr.c_str();
            // LOGW("%s", nameBrdStr);
            strlcpy(bestConfig.board, nameBrd, sizeof(bestConfig.board));
          }

          return &bestConfig;
        }
        else
        {
          zbOk = false;
          LOGD("Zigbee config ERROR");
        }
      }
    }

    LOGI("Config error with: %s", brdConfigs[brdIdx].board);
    // DynamicJsonDocument config(300);
    // config["searchId"] = brdIdx + 1;
    // writeDefaultConfig(configFileHw, config);

    snprintf(hwConfig.board, sizeof(hwConfig.board), "i%02d", brdIdx + 1);
    saveHwConfig(hwConfig);

    restartDevice();

    return nullptr;
  }

  if (bestConfig.eth.addr != -1)
  {
    LOGI("Returning best partial config");
    return &bestConfig;
  }
  else
  {
    LOGI("No valid config found, returning default");
    return &bestConfig;
  }
}

void wgBegin()
{
  // checkDNS();
  if (!wg.is_initialized())
  {

    const char *wg_preshared_key = nullptr;
    if (vpnCfg.wgPreSharedKey[0] != '\0')
    {
      wg_preshared_key = vpnCfg.wgPreSharedKey;
      LOGD("vpnCfg.wgPreSharedKey is used");
    }

    if (!wg.begin(
            vpnCfg.wgLocalIP,
            vpnCfg.wgLocalSubnet,
            vpnCfg.wgLocalPort,
            vpnCfg.wgLocalGateway,
            vpnCfg.wgLocalPrivKey,
            vpnCfg.wgEndAddr,
            vpnCfg.wgEndPubKey,
            vpnCfg.wgEndPort,
            vpnCfg.wgAllowedIP,
            vpnCfg.wgAllowedMask,
            vpnCfg.wgMakeDefault,
            wg_preshared_key))
    {
      printLogMsg(String("Failed to initialize WG"));
      vars.vpnWgInit = false;
    }
    else
    {
      printLogMsg(String("WG was initialized"));
      /*LOGD("%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",
           String(vpnCfg.wgLocalIP.toString().c_str()),
           String(vpnCfg.wgLocalSubnet.toString().c_str()),
           String(vpnCfg.wgLocalPort),
           String(vpnCfg.wgLocalGateway.toString().c_str()),
           String(vpnCfg.wgLocalPrivKey).c_str(),
           String(vpnCfg.wgEndAddr).c_str(),
           String(vpnCfg.wgEndPubKey).c_str(),
           String(vpnCfg.wgEndPort),
           String(vpnCfg.wgAllowedIP.toString().c_str()),
           String(vpnCfg.wgAllowedMask.toString().c_str()),
           String(vpnCfg.wgMakeDefault),
           String(wg_preshared_key).c_str());*/
      vars.vpnWgInit = true;
    }
  }
}

void wgLoop()
{

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
        uint16_t wgLocalPort = vpnCfg.wgLocalPort;
        vars.vpnWgCheckTime = millis() + 1000 * checkPeriod;
        if (wg.is_peer_up(&lwip_ip, &wgLocalPort))
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
          // vars.vpnWgPeerIp.clear();
          vars.vpnWgPeerIp.fromString("0.0.0.0");
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

int compareVersions(String v1, String v2)
{
  int v1_major = v1.substring(0, v1.indexOf('.')).toInt();
  int v2_major = v2.substring(0, v2.indexOf('.')).toInt();

  if (v1_major != v2_major)
  {
    return v1_major - v2_major;
  }

  String v1_suffix = v1.substring(v1.indexOf('.') + 1);
  String v2_suffix = v2.substring(v2.indexOf('.') + 1);

  if (v1_suffix.length() == 0)
    return -1;
  if (v2_suffix.length() == 0)
    return 1;

  return v1_suffix.compareTo(v2_suffix);
}

void checkUpdateAvail()
{
  if (!vars.apStarted)
  {
    const char *ESPkey = "ESP";
    const char *ZBkey = "ZB";
    const char *FoundKey = "Found ";
    const char *NewFwKey = " new fw: ";
    const char *TryKey = "try to install";
    // String latestReleaseUrlEsp = fetchLatestEspFw();
    String latestReleaseUrlZb = fetchLatestZbFw();

    // String latestVersionEsp = extractVersionFromURL(latestReleaseUrlEsp);

    FirmwareInfo esp_fw = fetchLatestEspFw();
    String latestVersionEsp = esp_fw.version;
    // String latestVersionEsp = "1.2.3";

    String latestVersionZb = extractVersionFromURL(latestReleaseUrlZb);

    strncpy(vars.lastESPVer, latestVersionEsp.c_str(), sizeof(vars.lastESPVer) - 1);
    vars.lastESPVer[sizeof(vars.lastESPVer) - 1] = '\0';

    LOGD("lastESPVer %s", vars.lastESPVer);

    strncpy(vars.lastZBVer, latestVersionZb.c_str(), sizeof(vars.lastZBVer) - 1);
    vars.lastZBVer[sizeof(vars.lastZBVer) - 1] = '\0';

    LOGD("lastZBVer %s", vars.lastZBVer);

    if (latestVersionEsp.length() > 0 && compareVersions(latestVersionEsp, VERSION) > 0)
    {
      vars.updateEspAvail = true;

      printLogMsg(String(FoundKey) + String(ESPkey) + String(NewFwKey) + latestVersionEsp);
      if (systemCfg.updAutoInst)
      {
        printLogMsg(String(TryKey));
        // getEspUpdate(latestReleaseUrlEsp);
      }
    }
    else
    {
      vars.updateEspAvail = false;
    }

    if (CCTool.chip.fwRev > 0 && latestVersionZb.length() > 0 && compareVersions(latestVersionZb, String(CCTool.chip.fwRev)) > 0)
    {
      vars.updateZbAvail = true;

      printLogMsg(String(FoundKey) + String(ZBkey) + String(NewFwKey) + latestVersionZb);
      if (systemCfg.updAutoInst)
      {
        printLogMsg(String(TryKey));
        // flashZbUrl(latestReleaseUrlZb);
        // restartDevice();
      }
    }
    else
    {
      vars.updateZbAvail = false;
    }
  }
  else
  {
    LOGW("AP is started, skip update check");
  }
}

bool isIpInSubnet(IPAddress ip, IPAddress subnetBase, IPAddress subnetMask)
{
  for (int i = 0; i < 4; i++)
  {
    if ((ip[i] & subnetMask[i]) != (subnetBase[i] & subnetMask[i]))
    {
      return false;
    }
  }
  return true;
}

bool isValidIp(IPAddress ip)
{
  return ip != IPAddress(0, 0, 0, 0);
}

String getHostFromUrl(const String &url)
{
  int startIndex = url.indexOf("://") + 3;
  int endIndex = url.indexOf('/', startIndex);
  if (endIndex == -1)
  {
    endIndex = url.length();
  }
  return url.substring(startIndex, endIndex);
}

String getRadioRoleKey()
{
  String role;
  switch (systemCfg.zbRole)
  {
  case COORDINATOR:
    role = "coordinator";
    break;
  case ROUTER:
    role = "router";
    break;
  case OPENTHREAD:
    role = "thread";
    break;
  default:
    role = "unknown";
    break;
  }
  return role;
}

// Функция для удаления ведущих нулей из блока IPv6
String removeLeadingZeros(const String &block)
{
  int i = 0;
  while (i < block.length() - 1 && block[i] == '0')
  {
    i++;
  }
  return block.substring(i);
}

// Функция для сокращения IPv6-адреса
String getShortenedIPv6(const String &ipv6)
{
  String result = "";
  int start = 0;
  int end = ipv6.indexOf(':');
  bool zeroBlock = false;
  bool inZeroBlock = false;

  while (end != -1)
  {
    String block = ipv6.substring(start, end);
    block = removeLeadingZeros(block);

    if (block == "0")
    {
      if (!inZeroBlock)
      {
        if (zeroBlock)
        {
          result += ":";
        }
        result += ":";
        inZeroBlock = true;
      }
    }
    else
    {
      if (inZeroBlock)
      {
        inZeroBlock = false;
      }
      else if (zeroBlock)
      {
        result += ":";
      }
      result += block;
      zeroBlock = true;
    }

    start = end + 1;
    end = ipv6.indexOf(':', start);
  }

  // Обработка последнего блока
  String block = ipv6.substring(start);
  block = removeLeadingZeros(block);
  if (block == "0")
  {
    if (!inZeroBlock)
    {
      if (zeroBlock)
      {
        result += ":";
      }
      result += ":";
    }
  }
  else
  {
    if (inZeroBlock)
    {
      inZeroBlock = false;
    }
    else if (zeroBlock)
    {
      result += ":";
    }
    result += block;
  }

  // Удаление лишнего двоеточия в начале, если есть
  if (result.startsWith("::") && result.length() > 2)
  {
    result = result.substring(1);
  }

  return result;
}

void restartDevice()
{
  LOGD("Restarting device...");
  delay(100);
  ESP.restart();
}

void freeHeapPrint()
{
  heap_caps_free(NULL);
  size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  size_t minFreeHeap = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
  LOGD("Free heap: %d, min free heap: %d", freeHeap, minFreeHeap);
}

bool dnsLookup(const String &url)
{
  IPAddress ip;
  String host = getHostFromUrl(url);

  // DNS lookup
  if (!WiFi.hostByName(host.c_str(), ip))
  {
    printLogMsg("DNS lookup failed for host: " + host + ". Check your network connection and DNS settings.");
    return false;
  }
  else
  {
    return true;
  }
}

void firstUpdCheck()
{
  int retryCount = 0;
  int maxRetries = 3;
  const int retryDelay = 500;
  bool isSuccess = false;

  while (retryCount < maxRetries && !isSuccess)
  {
    if (!dnsLookup("google.com"))
    {
      retryCount++;
      printLogMsg("DNS lookup failed. Retrying...");
      delay(retryDelay * 3);
    }
    else
    {
      isSuccess = true;
      vars.firstUpdCheck = true;
      checkUpdateAvail();
      checkFileSys();
    }
  }
}
