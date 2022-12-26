#include <Arduino.h>
#include <ArduinoJson.h>
#include <etc.h>
#include <WiFi.h>
#include "FS.h"
#include <LittleFS.h>

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
    DEBUG_PRINTLN(F("failed write"));
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
  digitalWrite(LED_BLUE, 1);
  digitalWrite(LED_YELLOW, 0);
  for (uint8_t i = 0; i < 15; i++){
    delay(200);
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
    digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
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