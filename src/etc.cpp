#include <Arduino.h>
#include <ArduinoJson.h>
#include <etc.h>
#include <WiFi.h>
#include "FS.h"
#include <LittleFS.h>
#include <ETH.h>
#include "config.h"
#include "log.h"
#include "web.h"

extern struct ConfigSettingsStruct ConfigSettings;
extern const char* deviceModel;

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

float readtemp(bool clear){
  if (clear == true) {
    return (temprature_sens_read() - 32) / 1.8;
  }
  else {
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
  }else{
    CPUtemp = readtemp(clear);
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
  printLogMsg("Zigbee Reset is done");
  DEBUG_PRINTLN(F("Zigbee Reset is done"));
}

void adapterModeUSB()
{
  printLogMsg("Switched UZG-01 to USB mode");
  DEBUG_PRINTLN(F("Switched UZG-01 to USB mode"));
  digitalWrite(MODE_SWITCH, 1);
  digitalWrite(LED_RED, 1);
}

void adapterModeLAN()
{
  printLogMsg("Switched UZG-01 to LAN mode");
  DEBUG_PRINTLN(F("Switched UZG-01 to LAN mode"));
  digitalWrite(MODE_SWITCH, 0);
  digitalWrite(LED_RED, 0);
}

void ledYellowToggle()
{
  printLogMsg("BLUE LED has been toggled");
  DEBUG_PRINTLN(F("BLUE LED has been toggled"));
  digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
}

void ledBlueToggle()
{
  printLogMsg("RED LED has been toggled");
  DEBUG_PRINTLN(F("RED LED has been toggled"));
  digitalWrite(LED_RED, !digitalRead(LED_RED));
}

void getDeviceID(char * arr){
  uint64_t mac = ESP.getEfuseMac();
  uint8_t a;
  uint8_t b;
  a ^= mac >> 8*0;
  a ^= mac >> 8*1;
  a ^= mac >> 8*2;
  a ^= mac >> 8*3;
  b ^= mac >> 8*4;
  b ^= mac >> 8*5;
  b ^= mac >> 8*6;
  b ^= mac >> 8*7;
  sprintf(arr, "%s--%3d%3d", deviceModel, a, b);
}

void getDeviceID_old(String &devID)
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
  devID = "ZigStarGW-" + String(mac);
  DEBUG_PRINTLN(devID);
}

// void writeDefultConfig(const char *path, String StringConfig)
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

void writeDefultConfig(const char *path, JsonDocument& doc){
  DEBUG_PRINTLN(path);
  DEBUG_PRINTLN(F("Write defaults"));

  File configFile = LittleFS.open(path, FILE_WRITE);
  if (!configFile){
    DEBUG_PRINTLN(F("Failed Write"));
    //return false;
  }else{
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

void resetSettings(){ 
  DEBUG_PRINTLN(F("[resetSettings] Start"));
  digitalWrite(LED_RED, 1);
  digitalWrite(LED_BLUE, 0);
  for (uint8_t i = 0; i < 15; i++){
    delay(200);
    digitalWrite(LED_RED, !digitalRead(LED_RED));
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
  }
  DEBUG_PRINTLN(F("[resetSettings] Led blinking done"));
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10)){
    DEBUG_PRINTLN(F("Error with LITTLEFS"));
  }
  LittleFS.remove("/config/configSerial.json");  //todo move 2 define or global const
  LittleFS.remove("/config/configSecurity.json");  //todo move 2 define or global const
  LittleFS.remove("/config/configGeneral.json");  //todo move 2 define or global const
  LittleFS.remove("/config/configEther.json");  //todo move 2 define or global const
  LittleFS.remove("/config/configWifi.json");  //todo move 2 define or global const
  LittleFS.remove("/config/system.json");  //todo move 2 define or global const
  DEBUG_PRINTLN(F("[resetSettings] Config del done"));
  ESP.restart();
}