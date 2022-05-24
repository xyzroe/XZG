#include <Arduino.h>
#include <ArduinoJson.h>
#include <etc.h>
#include <WiFi.h>
#include <ETH.h>
#include "LITTLEFS.h"

#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "web.h"

#include <OneWire.h>
#include <DallasTemperature.h>
#include <DS18B20.h>

OneWire oneWire(ONE_WIRE_BUS);

DS18B20 sensor(&oneWire);

extern struct ConfigSettingsStruct ConfigSettings;

void oneWireBegin()
{
  if (ConfigSettings.board == 2) {
    sensor.begin();
    DEBUG_PRINTLN(F("oneWire begin OK"));
  }
}

float oneWireRead()//String &DStemp)
{ 
  if (ConfigSettings.board == 2) {
    sensor.requestTemperatures();
    float tempC = sensor.getTempC();
    DEBUG_PRINTLN(tempC);
    if(tempC != DEVICE_DISCONNECTED_C && tempC != 0.0) 
    {
      DEBUG_PRINTLN(F("oneWire OK"));
      return tempC;
    } 
    else
    {
      DEBUG_PRINTLN(F("oneWire not found"));
      return false;
    }
  }
  else {
      DEBUG_PRINTLN(F("oneWire not supported"));
      return false;
  }
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

/*
void parse_ip_address(IPAddress &ip, const char *str)
{
  IPAddress result;
  int index = 0;

  result[0] = 0;
  while (*str)
  {
    if (isdigit((unsigned char)*str))
    {
      result[index] *= 10;
      result[index] += *str - '0';
    }
    else
    {
      index++;
      if (index < 4)
      {
        result[index] = 0;
      }
    }
    str++;
  }

  ip = result;
}
*/

void zigbeeEnableBSL()
{
  printLogMsg("Zigbee BSL pin ON");
  DEBUG_PRINTLN(F("Zigbee BSL pin ON"));
  digitalWrite(ConfigSettings.flashZigbeePin, 0);
  mqttPublishIo("enbl_bsl", "ON");
  delay(100);

  printLogMsg("Zigbee RST pin ON");
  DEBUG_PRINTLN(F("Zigbee RST pin ON"));
  digitalWrite(ConfigSettings.rstZigbeePin, 0);
  mqttPublishIo("rst_zig", "ON");
  delay(250);

  printLogMsg("Zigbee RST pin OFF");
  DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
  digitalWrite(ConfigSettings.rstZigbeePin, 1);
  mqttPublishIo("rst_zig", "OFF");
  delay(2000);

  printLogMsg("Zigbee BSL pin OFF");
  DEBUG_PRINTLN(F("Zigbee BSL pin OFF"));
  digitalWrite(ConfigSettings.flashZigbeePin, 1);
  mqttPublishIo("enbl_bsl", "OFF");
  printLogMsg("Now you can flash CC2652!");
}

void zigbeeRestart()
{
  printLogMsg("Zigbee RST pin ON");
  DEBUG_PRINTLN(F("Zigbee RST pin ON"));
  digitalWrite(ConfigSettings.rstZigbeePin, 0);
  mqttPublishIo("rst_zig", "ON");
  delay(250);
  printLogMsg("Zigbee RST pin OFF");
  DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
  digitalWrite(ConfigSettings.rstZigbeePin, 1);
  mqttPublishIo("rst_zig", "OFF");
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
  devID = "ZigStarGW-" + String(mac);
  DEBUG_PRINTLN(devID);
}

void writeDefultConfig(const char *path, String StringConfig)
{
  DEBUG_PRINTLN(path);
  DEBUG_PRINTLN(F("failed open. try to write defaults"));
  DEBUG_PRINTLN(StringConfig);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LITTLEFS.open(path, FILE_WRITE);
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