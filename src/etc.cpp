#include <Arduino.h>
#include <ArduinoJson.h>
#include <etc.h>
#include <WiFi.h>
#include <ETH.h>
#include "FS.h"
#include <LittleFS.h>

#include "config.h"
#include "log.h"
#include "web.h"

extern struct ConfigSettingsStruct ConfigSettings;

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

float getCPUtemp(bool clear)
{ 
  float CPUtemp = 0.0;
  if (!ConfigSettings.enableWiFi && !ConfigSettings.emergencyWifi)
  {
    WiFi.mode(WIFI_STA); // enable wifi to enable temp sensor
  }

  if (clear == true) {
    CPUtemp = (temprature_sens_read() - 32) / 1.8;
  }
  else {
    CPUtemp = (temprature_sens_read() - 32) / 1.8 - ConfigSettings.tempOffset;
  }
  
  if (!ConfigSettings.enableWiFi && !ConfigSettings.emergencyWifi)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF); // disable wifi
  }
  return CPUtemp;
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
  printLogMsg("Now you can flash CC2652!");
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
}

void adapterModeUSB()
{
  printLogMsg("Switched SLZB-06 to USB mode");
  DEBUG_PRINTLN(F("Switched SLZB-06 to USB mode"));
  digitalWrite(MODE_SWITCH, 1);
  digitalWrite(LED_BLUE, 1);
}

void adapterModeLAN()
{
  printLogMsg("Switched SLZB-06 to LAN mode");
  DEBUG_PRINTLN(F("Switched SLZB-06 to LAN mode"));
  digitalWrite(MODE_SWITCH, 0);
  digitalWrite(LED_BLUE, 0);
}

void ledYellowToggle()
{
  printLogMsg("Yellow LED has been toggled");
  DEBUG_PRINTLN(F("Yellow LED has been toggled"));
  digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
}

void ledBlueToggle()
{
  printLogMsg("Blue LED has been toggled");
  DEBUG_PRINTLN(F("Blue LED has been toggled"));
  digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
}

void getDeviceID(String &devID)
{
  String mac;
  mac = ETH.macAddress();
  if (strcmp(mac.c_str(), "00:00:00:00:00:00") != 0)
  {
    DEBUG_PRINTLN(F("Using ETH mac to ID"));
    DEBUG_PRINTLN(mac);
  }
  else
  {
    mac = WiFi.softAPmacAddress();
    if (strcmp(mac.c_str(), "") != 0)
    {
      DEBUG_PRINTLN(F("Using WIFI mac to ID"));
      DEBUG_PRINTLN(mac);
    }
    else
    {
      mac = "00:00:00:12:34:56";
      DEBUG_PRINTLN(F("Using zero mac to ID"));
      DEBUG_PRINTLN(mac);
    }
  }
  mac = mac.substring(9);
  mac = mac.substring(0, 2) + mac.substring(3, 5);
  devID = "SLZB-06--" + String(mac);
  DEBUG_PRINTLN(devID);
}

void writeDefultConfig(const char *path, String StringConfig)
{
  DEBUG_PRINTLN(path);
  DEBUG_PRINTLN(F("failed open. try to write defaults"));
  DEBUG_PRINTLN(StringConfig);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed write"));
    //return false;
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

void saveEmergencyWifi(bool state)
{ 
  DEBUG_PRINT(F("saveEmergencyWifi "));
  DEBUG_PRINTLN(state);

  const char *path = "/config/system.json";
  DynamicJsonDocument doc(1024);

  File configFile = LittleFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();

  doc["emergencyWifi"] = int(state);

  configFile = LittleFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
}


void saveRestartCount(int count)
{
  const char *path = "/config/system.json";
  DynamicJsonDocument doc(1024);

  File configFile = LittleFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();

  doc["restarts"] = int(count);

  configFile = LittleFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
  delay(500);
}

void resetSettings()
{ 

  const char *path = "/config/configSecurity.json";

  String StringConfig = "{\"disableWeb\":0,\"webAuth\":0,\"webUser\":"",\"webPass\":""}";

  writeDefultConfig(path, StringConfig);

  ESP.restart();
}