#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ETH.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "zones.h"

extern struct ConfigSettingsStruct ConfigSettings;

const char *coordMode = "coordMode";         // coordMode node name
const char *prevCoordMode = "prevCoordMode"; // prevCoordMode node name
const char *configFileSystem = "/config/system.json";
const char *configFileWifi = "/config/configWifi.json";
const char *configFileEther = "/config/configEther.json";
const char *configFileGeneral = "/config/configGeneral.json";
const char *configFileSecurity = "/config/configSecurity.json";
const char *configFileSerial = "/config/configSerial.json";
const char *configFileMqtt = "/config/configMqtt.json";
const char *configFileWg = "/config/configWg.json";
const char *deviceModel = "UZG-01";

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

float readtemp(bool clear)
{
  if (clear == true)
  {
    return (temprature_sens_read() - 32) / 1.8;
  }
  else
  {
    return (temprature_sens_read() - 32) / 1.8 - ConfigSettings.tempOffset;
  }
}

float getCPUtemp(bool clear)
{
  DEBUG_PRINTLN(F("getCPUtemp"));
  float CPUtemp = 0.0;
  if (WiFi.getMode() == WIFI_MODE_NULL || WiFi.getMode() == WIFI_OFF)
  {
    DEBUG_PRINTLN(F("enable wifi to enable temp sensor"));
    WiFi.mode(WIFI_STA); // enable wifi to enable temp sensor
    CPUtemp = readtemp(clear);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF); // disable wifi
  }
  else
  {
    CPUtemp = readtemp(clear);
  }
  return CPUtemp;
}

void zigbeeRouterRejoin()
{
  printLogMsg("Zigbee RST pin ON");
  DEBUG_PRINTLN(F("Zigbee RST pin ON"));
  digitalWrite(CC2652P_FLSH, 0);
  delay(250);

  printLogMsg("Zigbee RST pin OFF");
  DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
  digitalWrite(CC2652P_FLSH, 1);
  delay(500);

  printLogMsg("Router reconnect");
  DEBUG_PRINTLN(F("Router Reconnect"));
}

void zigbeeEnableBSL()
{
  printLogMsg("Zigbee BSL pin ON");
  DEBUG_PRINTLN(F("Zigbee BSL pin ON"));
  digitalWrite(CC2652P_FLSH, 0);
  delay(100);

  printLogMsg("Zigbee RST pin ON");
  DEBUG_PRINTLN(F("Zigbee RST pin ON"));
  digitalWrite(CC2652P_RST, 0);
  delay(250);

  printLogMsg("Zigbee RST pin OFF");
  DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
  digitalWrite(CC2652P_RST, 1);
  delay(2000);

  printLogMsg("Zigbee BSL pin OFF");
  DEBUG_PRINTLN(F("Zigbee BSL pin OFF"));
  digitalWrite(CC2652P_FLSH, 1);
  delay(4000);
  printLogMsg("Now you can flash CC2652!");
  DEBUG_PRINTLN(F("Now you can flash CC2652!"));
}

void zigbeeRestart()
{
  printLogMsg("Zigbee RST pin ON");
  DEBUG_PRINTLN(F("Zigbee RST pin ON"));
  digitalWrite(CC2652P_RST, 0);
  delay(250);
  printLogMsg("Zigbee RST pin OFF");
  DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
  digitalWrite(CC2652P_RST, 1);
  delay(2000);
  printLogMsg("Zigbee restart was done");
  DEBUG_PRINTLN(F("Zigbee restart was done"));
}

void adapterModeUSB()
{
  printLogMsg("Switched UZG-01 to USB mode");
  DEBUG_PRINTLN(F("Switched UZG-01 to USB mode"));
  digitalWrite(MODE_SWITCH, 1);
  digitalWrite(LED_USB, 1);
}

void adapterModeLAN()
{
  printLogMsg("Switched UZG-01 to LAN mode");
  DEBUG_PRINTLN(F("Switched UZG-01 to LAN mode"));
  digitalWrite(MODE_SWITCH, 0);
  digitalWrite(LED_USB, 0);
}

void ledPowerToggle()
{
  printLogMsg("BLUE LED has been toggled");
  DEBUG_PRINTLN(F("BLUE LED has been toggled"));
  digitalWrite(LED_PWR, !digitalRead(LED_PWR));
}

void ledUSBToggle()
{
  printLogMsg("RED LED has been toggled");
  DEBUG_PRINTLN(F("RED LED has been toggled"));
  digitalWrite(LED_USB, !digitalRead(LED_USB));
}

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
  sprintf(arr, "%s-%s", deviceModel, buf);
  // arr = buf;
}

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
  DEBUG_PRINTLN(path);
  DEBUG_PRINTLN(F("Write defaults"));

  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("Failed Write"));
    // return false;
  }
  else
  {
    serializeJson(doc, configFile);
  }
  configFile.close();
}

String hexToDec(String hexString)
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
}

void resetSettings()
{
  DEBUG_PRINTLN(F("[resetSettings] Start"));
  digitalWrite(LED_USB, 1);
  digitalWrite(LED_PWR, 0);
  for (uint8_t i = 0; i < 15; i++)
  {
    delay(200);
    digitalWrite(LED_USB, !digitalRead(LED_USB));
    digitalWrite(LED_PWR, !digitalRead(LED_PWR));
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
  DEBUG_PRINTLN(F("[resetSettings] Config del done"));
  ESP.restart();
}

void setClock()
{
  configTime(0, 0, "pool.ntp.org", "time.google.com");

  DEBUG_PRINT(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while ((nowSecs < 60))
  {
    delay(500);
    DEBUG_PRINT(F("."));
    yield();
    nowSecs = time(nullptr);
    DEBUG_PRINT(nowSecs);
  }

  DEBUG_PRINTLN();
  struct tm timeinfo;
  localtime_r(&nowSecs, &timeinfo);
  DEBUG_PRINT(F("Current GMT time: "));
  DEBUG_PRINT(asctime(&timeinfo));

  char *zoneToFind = "Europe/Kiev";
  if (ConfigSettings.timeZone)
  {
    zoneToFind = ConfigSettings.timeZone;
  }
  const char *gmtOffset = getGmtOffsetForZone(zoneToFind);

  String timezone = "EET-2EEST,M3.5.0/3,M10.5.0/4";

  if (gmtOffset != nullptr)
  {
    DEBUG_PRINT("GMT Offset for ");
    DEBUG_PRINT(zoneToFind);
    DEBUG_PRINT(" is ");
    DEBUG_PRINTLN(gmtOffset);
    timezone = gmtOffset;
    setTimezone(timezone);
  }
  else
  {
    DEBUG_PRINT("GMT Offset for ");
    DEBUG_PRINT(zoneToFind);
    DEBUG_PRINTLN(" not found.");
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
  DEBUG_PRINT(F("Local time: "));
  DEBUG_PRINTLN(asctime(&timeinfo));
}

const char *getGmtOffsetForZone(const char *zone)
{
  for (int i = 0; i < timeZoneCount; i++)
  {
    if (strcmp(zone, timeZones[i].zone) == 0)
    {
      // Зона найдена, возвращаем GMT Offset
      return timeZones[i].gmtOffset;
    }
  }
  // Зона не найдена
  return nullptr;
}
